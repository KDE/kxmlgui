/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-xmlgui.h"

#include "kkeysequencewidget.h"
#include "kkeysequencewidget_p.h"
#include "debug.h"

#include <QAction>
#include <QKeyEvent>
#include <QTimer>
#include <QHash>
#include <QHBoxLayout>
#include <QToolButton>
#include <QApplication>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kkeyserver.h>
#include <KWindowSystem>
#if HAVE_GLOBALACCEL
# include <kglobalaccel.h>
#endif

#include "kactioncollection.h"
#include "waylandshortcutinhibitor.h"

#include <memory>

class KKeySequenceWidgetPrivate
{
public:
    KKeySequenceWidgetPrivate(KKeySequenceWidget *q);

    void init();

    static QKeySequence appendToSequence(const QKeySequence &seq, int keyQt);
    static bool isOkWhenModifierless(int keyQt);

    void updateShortcutDisplay();
    void startRecording();

    /**
     * Conflicts the key sequence @p seq with a current standard
     * shortcut?
     */
    bool conflictWithStandardShortcuts(const QKeySequence &seq);

    /**
     * Conflicts the key sequence @p seq with a current local
     * shortcut?
     */
    bool conflictWithLocalShortcuts(const QKeySequence &seq);

    /**
     * Conflicts the key sequence @p seq with a current global
     * shortcut?
     */
    bool conflictWithGlobalShortcuts(const QKeySequence &seq);

    /**
     * Get permission to steal the shortcut @seq from the standard shortcut @p std.
     */
    bool stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq);

    bool checkAgainstStandardShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts;
    }

    bool checkAgainstGlobalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts;
    }

    bool checkAgainstLocalShortcuts() const
    {
        return checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts;
    }

    void controlModifierlessTimout()
    {
        if (nKey != 0 && !modifierKeys) {
            // No modifier key pressed currently. Start the timout
            modifierlessTimeout.start(600);
        } else {
            // A modifier is pressed. Stop the timeout
            modifierlessTimeout.stop();
        }

    }

    void cancelRecording()
    {
        keySequence = oldKeySequence;
        doneRecording();
    }

#if HAVE_GLOBALACCEL
    bool promptStealShortcutSystemwide(
        QWidget *parent,
        const QHash<QKeySequence, QList<KGlobalShortcutInfo> > &shortcuts,
        const QKeySequence &sequence)
    {
        if (shortcuts.isEmpty()) {
            // Usage error. Just say no
            return false;
        }

        QString clashingKeys;
        for (auto it = shortcuts.begin(), end = shortcuts.end(); it != end; ++it) {
            const auto seqAsString = it.key().toString();
            for (const KGlobalShortcutInfo &info : it.value()) {
                clashingKeys += i18n("Shortcut '%1' in Application %2 for action %3\n",
                                     seqAsString,
                                     info.componentFriendlyName(),
                                     info.friendlyName());
            }
        }

        const int hashSize = shortcuts.size();

        QString message = i18ncp("%1 is the number of conflicts (hidden), %2 is the key sequence of the shortcut that is problematic",
                                 "The shortcut '%2' conflicts with the following key combination:\n",
                                 "The shortcut '%2' conflicts with the following key combinations:\n",
                                 hashSize, sequence.toString());
        message += clashingKeys;

        QString title = i18ncp("%1 is the number of shortcuts with which there is a conflict",
                               "Conflict with Registered Global Shortcut", "Conflict with Registered Global Shortcuts", hashSize);

        return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18nc("@action:button", "Reassign")))
               == KMessageBox::Continue;
    }
#endif

//private slot
    void doneRecording(bool validate = true);

//members
    KKeySequenceWidget *const q;
    QHBoxLayout *layout;
    KKeySequenceButton *keyButton;
    QToolButton *clearButton;

    QKeySequence keySequence;
    QKeySequence oldKeySequence;
    QTimer modifierlessTimeout;
    bool allowModifierless;
    uint nKey;
    uint modifierKeys;
    bool isRecording;
    bool multiKeyShortcutsAllowed;
    QString componentName;
    std::unique_ptr<WaylandShortcutInhibitor> shortcutInhibitor;

    //! Check the key sequence against KStandardShortcut::find()
    KKeySequenceWidget::ShortcutTypes checkAgainstShortcutTypes;

    /**
     * The list of action to check against for conflict shortcut
     */
    QList<QAction *> checkList; // deprecated

    /**
     * The list of action collections to check against for conflict shortcut
     */
    QList<KActionCollection *> checkActionCollections;

    /**
     * The action to steal the shortcut from.
     */
    QList<QAction *> stealActions;

    bool stealShortcuts(const QList<QAction *> &actions, const QKeySequence &seq);
    void wontStealShortcut(QAction *item, const QKeySequence &seq);

};

KKeySequenceWidgetPrivate::KKeySequenceWidgetPrivate(KKeySequenceWidget *q)
    : q(q)
    , layout(nullptr)
    , keyButton(nullptr)
    , clearButton(nullptr)
    , allowModifierless(false)
    , nKey(0)
    , modifierKeys(0)
    , isRecording(false)
    , multiKeyShortcutsAllowed(true)
    , componentName()
    , checkAgainstShortcutTypes(KKeySequenceWidget::LocalShortcuts | KKeySequenceWidget::GlobalShortcuts)
    , stealActions()
    , shortcutInhibitor(nullptr)
{
}

bool KKeySequenceWidgetPrivate::stealShortcuts(
    const QList<QAction *> &actions,
    const QKeySequence &seq)
{

    const int listSize = actions.size();

    QString title = i18ncp("%1 is the number of conflicts", "Shortcut Conflict", "Shortcut Conflicts", listSize);

    QString conflictingShortcuts;
    for (const QAction *action : actions) {
        conflictingShortcuts += i18n("Shortcut '%1' for action '%2'\n",
                                     action->shortcut().toString(QKeySequence::NativeText),
                                     KLocalizedString::removeAcceleratorMarker(action->text()));
    }
    QString message = i18ncp("%1 is the number of ambigious shortcut clashes (hidden)",
                             "The \"%2\" shortcut is ambiguous with the following shortcut.\n"
                             "Do you want to assign an empty shortcut to this action?\n"
                             "%3",
                             "The \"%2\" shortcut is ambiguous with the following shortcuts.\n"
                             "Do you want to assign an empty shortcut to these actions?\n"
                             "%3",
                             listSize,
                             seq.toString(QKeySequence::NativeText),
                             conflictingShortcuts);

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18nc("@action:button", "Reassign"))) != KMessageBox::Continue) {
        return false;
    }

    return true;
}

void KKeySequenceWidgetPrivate::wontStealShortcut(QAction *item, const QKeySequence &seq)
{
    QString title(i18nc("@title:window", "Shortcut conflict"));
    QString msg(i18n("<qt>The '%1' key combination is already used by the <b>%2</b> action.<br>"
                     "Please select a different one.</qt>", seq.toString(QKeySequence::NativeText),
                     KLocalizedString::removeAcceleratorMarker(item->text())));
    KMessageBox::sorry(q, msg, title);
}

KKeySequenceWidget::KKeySequenceWidget(QWidget *parent)
    : QWidget(parent),
      d(new KKeySequenceWidgetPrivate(this))
{
    d->init();
    setFocusProxy(d->keyButton);
    connect(d->keyButton, &KKeySequenceButton::clicked, this, &KKeySequenceWidget::captureKeySequence);
    connect(d->clearButton, &QToolButton::clicked, this, &KKeySequenceWidget::clearKeySequence);
    connect(&d->modifierlessTimeout, SIGNAL(timeout()), this, SLOT(doneRecording()));
    //TODO: how to adopt style changes at runtime?
    /*QFont modFont = d->clearButton->font();
    modFont.setStyleHint(QFont::TypeWriter);
    d->clearButton->setFont(modFont);*/
    d->updateShortcutDisplay();
}

void KKeySequenceWidgetPrivate::init()
{
    layout = new QHBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);

    keyButton = new KKeySequenceButton(this, q);
    keyButton->setFocusPolicy(Qt::StrongFocus);
    keyButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    keyButton->setToolTip(i18nc("@info:tooltip", "Click on the button, then enter the shortcut like you would in the program.\nExample for Ctrl+A: hold the Ctrl key and press A."));
    layout->addWidget(keyButton);

    clearButton = new QToolButton(q);
    layout->addWidget(clearButton);

    if (qApp->isLeftToRight()) {
        clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-rtl")));
    } else {
        clearButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-locationbar-ltr")));
    }

    if (KWindowSystem::isPlatformWayland()) {
        shortcutInhibitor.reset(new WaylandShortcutInhibitor(q->window()->windowHandle()));
    }
}

KKeySequenceWidget::~KKeySequenceWidget()
{
    delete d;
}

KKeySequenceWidget::ShortcutTypes KKeySequenceWidget::checkForConflictsAgainst() const
{
    return d->checkAgainstShortcutTypes;
}

void KKeySequenceWidget::setComponentName(const QString &componentName)
{
    d->componentName = componentName;
}

bool KKeySequenceWidget::multiKeyShortcutsAllowed() const
{
    return d->multiKeyShortcutsAllowed;
}

void KKeySequenceWidget::setMultiKeyShortcutsAllowed(bool allowed)
{
    d->multiKeyShortcutsAllowed = allowed;
}

void KKeySequenceWidget::setCheckForConflictsAgainst(ShortcutTypes types)
{
    d->checkAgainstShortcutTypes = types;
}

void KKeySequenceWidget::setModifierlessAllowed(bool allow)
{
    d->allowModifierless = allow;
}

bool KKeySequenceWidget::isKeySequenceAvailable(const QKeySequence &keySequence) const
{
    if (keySequence.isEmpty()) {
        return true;
    }
    return !(d->conflictWithLocalShortcuts(keySequence)
             || d->conflictWithGlobalShortcuts(keySequence)
             || d->conflictWithStandardShortcuts(keySequence));
}

bool KKeySequenceWidget::isModifierlessAllowed()
{
    return d->allowModifierless;
}

void KKeySequenceWidget::setClearButtonShown(bool show)
{
    d->clearButton->setVisible(show);
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(4, 1)
void KKeySequenceWidget::setCheckActionList(const QList<QAction *> &checkList) // deprecated
{
    d->checkList = checkList;
    Q_ASSERT(d->checkActionCollections.isEmpty()); // don't call this method if you call setCheckActionCollections!
}
#endif

void KKeySequenceWidget::setCheckActionCollections(const QList<KActionCollection *> &actionCollections)
{
    d->checkActionCollections = actionCollections;
}

//slot
void KKeySequenceWidget::captureKeySequence()
{
    d->startRecording();
}

QKeySequence KKeySequenceWidget::keySequence() const
{
    return d->keySequence;
}

//slot
void KKeySequenceWidget::setKeySequence(const QKeySequence &seq, Validation validate)
{
    // oldKeySequence holds the key sequence before recording started, if setKeySequence()
    // is called while not recording then set oldKeySequence to the existing sequence so
    // that the keySequenceChanged() signal is emitted if the new and previous key
    // sequences are different
    if (!d->isRecording) {
        d->oldKeySequence = d->keySequence;
    }

    d->keySequence = seq;
    d->doneRecording(validate == Validate);
}

//slot
void KKeySequenceWidget::clearKeySequence()
{
    setKeySequence(QKeySequence());
}

//slot
void KKeySequenceWidget::applyStealShortcut()
{
    QSet<KActionCollection *> changedCollections;

    for (QAction *stealAction : qAsConst(d->stealActions)) {

        // Stealing a shortcut means setting it to an empty one.
        stealAction->setShortcuts(QList<QKeySequence>());

        // The following code will find the action we are about to
        // steal from and save it's actioncollection.
        KActionCollection *parentCollection = nullptr;
        for (KActionCollection *collection : qAsConst(d->checkActionCollections)) {
            if (collection->actions().contains(stealAction)) {
                parentCollection = collection;
                break;
            }
        }

        // Remember the changed collection
        if (parentCollection) {
            changedCollections.insert(parentCollection);
        }
    }

    for (KActionCollection *col : qAsConst(changedCollections)) {
        col->writeSettings();
    }

    d->stealActions.clear();
}

void KKeySequenceWidgetPrivate::startRecording()
{
    nKey = 0;
    modifierKeys = 0;
    oldKeySequence = keySequence;
    keySequence = QKeySequence();
    isRecording = true;
    keyButton->grabKeyboard();
    if (shortcutInhibitor) {
        shortcutInhibitor->enableInhibition();
    }

    if (!QWidget::keyboardGrabber()) {
        qCWarning(DEBUG_KXMLGUI) << "Failed to grab the keyboard! Most likely qt's nograb option is active";
    }

    keyButton->setDown(true);
    updateShortcutDisplay();
}

void KKeySequenceWidgetPrivate::doneRecording(bool validate)
{
    modifierlessTimeout.stop();
    isRecording = false;
    keyButton->releaseKeyboard();
    keyButton->setDown(false);
    stealActions.clear();
    if (shortcutInhibitor) {
        shortcutInhibitor->disableInhibition();
    }

    if (keySequence == oldKeySequence) {
        // The sequence hasn't changed
        updateShortcutDisplay();
        return;
    }

    if (validate && !q->isKeySequenceAvailable(keySequence)) {
        // The sequence had conflicts and the user said no to stealing it
        keySequence = oldKeySequence;
    } else {
        emit q->keySequenceChanged(keySequence);
    }

    updateShortcutDisplay();
}

bool KKeySequenceWidgetPrivate::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
#ifdef Q_OS_WIN
    //on windows F12 is reserved by the debugger at all times, so we can't use it for a global shortcut
    if (KKeySequenceWidget::GlobalShortcuts && keySequence.toString().contains(QLatin1String("F12"))) {
        QString title = i18n("Reserved Shortcut");
        QString message = i18n("The F12 key is reserved on Windows, so cannot be used for a global shortcut.\n"
                               "Please choose another one.");

        KMessageBox::sorry(q, message, title);
        return false;
    }
#endif

#if HAVE_GLOBALACCEL
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::GlobalShortcuts)) {
        return false;
    }

    // Global shortcuts are on key+modifier shortcuts. They can clash with
    // each of the keys of a multi key shortcut.
    QHash<QKeySequence, QList<KGlobalShortcutInfo> > others;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence tmp(keySequence[i]);

        if (!KGlobalAccel::isGlobalShortcutAvailable(tmp, componentName)) {
            others.insert(tmp, KGlobalAccel::getGlobalShortcutsByKey(tmp));
        }
    }

    if (!others.isEmpty()
            && !promptStealShortcutSystemwide(q, others, keySequence)) {
        return true;
    }

    // The user approved stealing the shortcut. We have to steal
    // it immediately because KAction::setGlobalShortcut() refuses
    // to set a global shortcut that is already used. There is no
    // error it just silently fails. So be nice because this is
    // most likely the first action that is done in the slot
    // listening to keySequenceChanged().
    for (int i = 0; i < keySequence.count(); ++i) {
        KGlobalAccel::stealShortcutSystemwide(keySequence[i]);
    }
    return false;
#else
    Q_UNUSED(keySequence);
    return false;
#endif
}

static bool shortcutsConflictWith(const QList<QKeySequence> &shortcuts, const QKeySequence &needle)
{
    if (needle.isEmpty()) {
        return false;
    }

    for (const QKeySequence &sequence : shortcuts) {
        if (sequence.isEmpty()) {
            continue;
        }

        if (sequence.matches(needle) != QKeySequence::NoMatch
                || needle.matches(sequence) != QKeySequence::NoMatch) {
            return true;
        }
    }

    return false;
}

bool KKeySequenceWidgetPrivate::conflictWithLocalShortcuts(const QKeySequence &keySequence)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::LocalShortcuts)) {
        return false;
    }

    // We have actions both in the deprecated checkList and the
    // checkActionCollections list. Add all the actions to a single list to
    // be able to process them in a single loop below.
    // Note that this can't be done in setCheckActionCollections(), because we
    // keep pointers to the action collections, and between the call to
    // setCheckActionCollections() and this function some actions might already be
    // removed from the collection again.
    QList<QAction *> allActions;
    allActions += checkList;
    for (KActionCollection *collection : qAsConst(checkActionCollections)) {
        allActions += collection->actions();
    }

    // Because of multikey shortcuts we can have clashes with many shortcuts.
    //
    // Example 1:
    //
    // Application currently uses 'CTRL-X,a', 'CTRL-X,f' and 'CTRL-X,CTRL-F'
    // and the user wants to use 'CTRL-X'. 'CTRL-X' will only trigger as
    // 'activatedAmbiguously()' for obvious reasons.
    //
    // Example 2:
    //
    // Application currently uses 'CTRL-X'. User wants to use 'CTRL-X,CTRL-F'.
    // This will shadow 'CTRL-X' for the same reason as above.
    //
    // Example 3:
    //
    // Some weird combination of Example 1 and 2 with three shortcuts using
    // 1/2/3 key shortcuts. I think you can imagine.
    QList<QAction *> conflictingActions;

    //find conflicting shortcuts with existing actions
    for (QAction *qaction : qAsConst(allActions)) {
        if (shortcutsConflictWith(qaction->shortcuts(), keySequence)) {
            // A conflict with a KAction. If that action is configurable
            // ask the user what to do. If not reject this keySequence.
            if (checkActionCollections.first()->isShortcutsConfigurable(qaction)) {
                conflictingActions.append(qaction);
            } else {
                wontStealShortcut(qaction, keySequence);
                return true;
            }
        }
    }

    if (conflictingActions.isEmpty()) {
        // No conflicting shortcuts found.
        return false;
    }

    if (stealShortcuts(conflictingActions, keySequence)) {
        stealActions = conflictingActions;
        // Announce that the user
        // agreed
        for (QAction *stealAction : qAsConst(stealActions)) {
            emit q->stealShortcut(
                keySequence,
                stealAction);
        }
        return false;
    } else {
        return true;
    }
}

bool KKeySequenceWidgetPrivate::conflictWithStandardShortcuts(const QKeySequence &keySequence)
{
    if (!(checkAgainstShortcutTypes & KKeySequenceWidget::StandardShortcuts)) {
        return false;
    }

    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
    if (ssc != KStandardShortcut::AccelNone && !stealStandardShortcut(ssc, keySequence)) {
        return true;
    }
    return false;
}

bool KKeySequenceWidgetPrivate::stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
    QString title = i18nc("@title:window", "Conflict with Standard Application Shortcut");
    QString message = i18n("The '%1' key combination is also used for the standard action "
                           "\"%2\" that some applications use.\n"
                           "Do you really want to use it as a global shortcut as well?",
                           seq.toString(QKeySequence::NativeText), KStandardShortcut::label(std));

    if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18nc("@action:button", "Reassign"))) != KMessageBox::Continue) {
        return false;
    }
    return true;
}

void KKeySequenceWidgetPrivate::updateShortcutDisplay()
{
    //empty string if no non-modifier was pressed
    QString s = keySequence.toString(QKeySequence::NativeText);
    s.replace(QLatin1Char('&'), QLatin1String("&&"));

    if (isRecording) {
        if (modifierKeys) {
            if (!s.isEmpty()) {
                s.append(QLatin1Char(','));
            }
            if (modifierKeys & Qt::MetaModifier) {
                s += QKeySequence(Qt::MetaModifier).toString(QKeySequence::NativeText);
            }
#if defined(Q_OS_MAC)
            if (modifierKeys & Qt::AltModifier) {
                s += QKeySequence(Qt::AltModifier).toString(QKeySequence::NativeText);
            }
            if (modifierKeys & Qt::ControlModifier) {
                s += QKeySequence(Qt::ControlModifier).toString(QKeySequence::NativeText);
            }
#else
            if (modifierKeys & Qt::ControlModifier) {
                s += QKeySequence(Qt::ControlModifier).toString(QKeySequence::NativeText);
            }
            if (modifierKeys & Qt::AltModifier) {
                s += QKeySequence(Qt::AltModifier).toString(QKeySequence::NativeText);
            }
#endif
            if (modifierKeys & Qt::ShiftModifier) {
                s += QKeySequence(Qt::ShiftModifier).toString(QKeySequence::NativeText);
            }
            if (modifierKeys & Qt::KeypadModifier) {
                s += QKeySequence(Qt::KeypadModifier).toString(QKeySequence::NativeText);
            }

        } else if (nKey == 0) {
            s = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
        }
        //make it clear that input is still going on
        s.append(QLatin1String(" ..."));
    }

    if (s.isEmpty()) {
        s = i18nc("No shortcut defined", "None");
    }

    s = QLatin1Char(' ') + s + QLatin1Char(' ');
    keyButton->setText(s);
}

KKeySequenceButton::~KKeySequenceButton()
{
}

//prevent Qt from special casing Tab and Backtab
bool KKeySequenceButton::event(QEvent *e)
{
    if (d->isRecording && e->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(e));
        return true;
    }

    // The shortcut 'alt+c' ( or any other dialog local action shortcut )
    // ended the recording and triggered the action associated with the
    // action. In case of 'alt+c' ending the dialog.  It seems that those
    // ShortcutOverride events get sent even if grabKeyboard() is active.
    if (d->isRecording && e->type() == QEvent::ShortcutOverride) {
        e->accept();
        return true;
    }

    if (d->isRecording && e->type() == QEvent::ContextMenu) {
        // is caused by Qt::Key_Menu
        e->accept();
        return true;
    }

    return QPushButton::event(e);
}

void KKeySequenceButton::keyPressEvent(QKeyEvent *e)
{
    int keyQt = e->key();
    if (keyQt == -1) {
        // Qt sometimes returns garbage keycodes, I observed -1, if it doesn't know a key.
        // We cannot do anything useful with those (several keys have -1, indistinguishable)
        // and QKeySequence.toString() will also yield a garbage string.
        KMessageBox::sorry(this,
                           i18n("The key you just pressed is not supported by Qt."),
                           i18nc("@title:window", "Unsupported Key"));
        d->cancelRecording();
        return;
    }

    uint newModifiers = e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);


    // Qt doesn't properly recognize Super_L/Super_R as MetaModifier
    if (e->key() == Qt::Key_Super_L || e->key() == Qt::Key_Super_R) {
        newModifiers |= Qt::MetaModifier;
    }

    //don't have the return or space key appear as first key of the sequence when they
    //were pressed to start editing - catch and them and imitate their effect
    if (!d->isRecording && ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space))) {
        d->startRecording();
        d->modifierKeys = newModifiers;
        d->updateShortcutDisplay();
        return;
    }

    // We get events even if recording isn't active.
    if (!d->isRecording) {
        QPushButton::keyPressEvent(e);
        return;
    }

    e->accept();
    d->modifierKeys = newModifiers;

    switch (keyQt) {
    case Qt::Key_AltGr: //or else we get unicode salad
        return;
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
        break;
    default:

        if (d->nKey == 0 && !(d->modifierKeys & ~Qt::SHIFT)) {
            // It's the first key and no modifier pressed. Check if this is
            // allowed
            if (!(KKeySequenceWidgetPrivate::isOkWhenModifierless(keyQt)
                    || d->allowModifierless)) {
                // No it's not
                return;
            }
        }

        // We now have a valid key press.
        if (keyQt) {
            if ((keyQt == Qt::Key_Backtab) && (d->modifierKeys & Qt::SHIFT)) {
                keyQt = Qt::Key_Tab | d->modifierKeys;
            } else if (KKeyServer::isShiftAsModifierAllowed(keyQt)) {
                keyQt |= d->modifierKeys;
            } else {
                keyQt |= (d->modifierKeys & ~Qt::SHIFT);
            }

            if (d->nKey == 0) {
                d->keySequence = QKeySequence(keyQt);
            } else {
                d->keySequence =
                    KKeySequenceWidgetPrivate::appendToSequence(d->keySequence, keyQt);
            }

            d->nKey++;
            if ((!d->multiKeyShortcutsAllowed) || (d->nKey >= 4)) {
                d->doneRecording();
                return;
            }
            d->controlModifierlessTimout();
            d->updateShortcutDisplay();
        }
    }
}

void KKeySequenceButton::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == -1) {
        // ignore garbage, see keyPressEvent()
        return;
    }

    if (!d->isRecording) {
        QPushButton::keyReleaseEvent(e);
        return;
    }

    e->accept();

    uint newModifiers = e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);

    // Qt doesn't properly recognize Super_L/Super_R as MetaModifier
    if (e->key() == Qt::Key_Super_L || e->key() == Qt::Key_Super_R) {
        newModifiers &= ~Qt::MetaModifier;
    }

    //if a modifier that belongs to the shortcut was released...
    if ((newModifiers & d->modifierKeys) < d->modifierKeys) {
        d->modifierKeys = newModifiers;
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
    }
}

//static
QKeySequence KKeySequenceWidgetPrivate::appendToSequence(const QKeySequence &seq, int keyQt)
{
    switch (seq.count()) {
    case 0:
        return QKeySequence(keyQt);
    case 1:
        return QKeySequence(seq[0], keyQt);
    case 2:
        return QKeySequence(seq[0], seq[1], keyQt);
    case 3:
        return QKeySequence(seq[0], seq[1], seq[2], keyQt);
    default:
        return seq;
    }
}

//static
bool KKeySequenceWidgetPrivate::isOkWhenModifierless(int keyQt)
{
    //this whole function is a hack, but especially the first line of code
    if (QKeySequence(keyQt).toString().length() == 1) {
        return false;
    }

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: //does this ever happen?
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return false;
    default:
        return true;
    }
}

#include "moc_kkeysequencewidget.cpp"
