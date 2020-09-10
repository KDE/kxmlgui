/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kxmlguiwindow.h"
#include "debug.h"

#include "kmainwindow_p.h"
#include "kmessagebox.h"
#include "kactioncollection.h"
#ifdef QT_DBUS_LIB
#include "kmainwindowiface_p.h"
#endif
#include "ktoolbarhandler_p.h"
#include "kxmlguifactory.h"
#include "kedittoolbar.h"
#include "khelpmenu.h"
#include "ktoolbar.h"

#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#endif
#include <QDomDocument>
#include <QMenuBar>
#include <QStatusBar>
#include <QWidget>
#include <QList>
#include <QEvent>

#include <ktoggleaction.h>
#include <kstandardaction.h>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kaboutdata.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <stdlib.h>
#include <ctype.h>

class KXmlGuiWindowPrivate : public KMainWindowPrivate
{
public:
    void _k_slotFactoryMakingChanges(bool b)
    {
        // While the GUI factory is adding/removing clients,
        // don't let KMainWindow think those are changes made by the user
        // #105525
        letDirtySettings = !b;
    }

    bool showHelpMenu: 1;
    QSize defaultSize;

    KDEPrivate::ToolBarHandler *toolBarHandler;
    KToggleAction *showStatusBarAction;
    QPointer<KEditToolBar> toolBarEditor;
    KXMLGUIFactory *factory;
};

KXmlGuiWindow::KXmlGuiWindow(QWidget *parent, Qt::WindowFlags f)
    : KMainWindow(*new KXmlGuiWindowPrivate, parent, f), KXMLGUIBuilder(this)
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = true;
    d->toolBarHandler = nullptr;
    d->showStatusBarAction = nullptr;
    d->factory = nullptr;
#ifdef QT_DBUS_LIB
    new KMainWindowInterface(this);
#endif
}

QAction *KXmlGuiWindow::toolBarMenuAction()
{
    K_D(KXmlGuiWindow);
    if (!d->toolBarHandler) {
        return nullptr;
    }

    return d->toolBarHandler->toolBarMenuAction();
}

void KXmlGuiWindow::setupToolbarMenuActions()
{
    K_D(KXmlGuiWindow);
    if (d->toolBarHandler) {
        d->toolBarHandler->setupActions();
    }
}

KXmlGuiWindow::~KXmlGuiWindow()
{
    K_D(KXmlGuiWindow);
    delete d->factory;
}

bool KXmlGuiWindow::event(QEvent *ev)
{
    bool ret = KMainWindow::event(ev);
    if (ev->type() == QEvent::Polish) {
#ifdef QT_DBUS_LIB
        QDBusConnection::sessionBus().registerObject(dbusName() + QLatin1String("/actions"), actionCollection(),
                QDBusConnection::ExportScriptableSlots |
                QDBusConnection::ExportScriptableProperties |
                QDBusConnection::ExportNonScriptableSlots |
                QDBusConnection::ExportNonScriptableProperties |
                QDBusConnection::ExportChildObjects);
#endif
    }
    return ret;
}

void KXmlGuiWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = showHelpMenu;
}

bool KXmlGuiWindow::isHelpMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return d->showHelpMenu;
}

KXMLGUIFactory *KXmlGuiWindow::guiFactory()
{
    K_D(KXmlGuiWindow);
    if (!d->factory) {
        d->factory = new KXMLGUIFactory(this, this);
        connect(d->factory, SIGNAL(makingChanges(bool)),
                this, SLOT(_k_slotFactoryMakingChanges(bool)));
    }
    return d->factory;
}

void KXmlGuiWindow::configureToolbars()
{
    K_D(KXmlGuiWindow);
    KConfigGroup cg(KSharedConfig::openConfig(), "");
    saveMainWindowSettings(cg);
    if (!d->toolBarEditor) {
        d->toolBarEditor = new KEditToolBar(guiFactory(), this);
        d->toolBarEditor->setAttribute(Qt::WA_DeleteOnClose);
        connect(d->toolBarEditor, &KEditToolBar::newToolBarConfig,
                this, &KXmlGuiWindow::saveNewToolbarConfig);
    }
    d->toolBarEditor->show();
}

void KXmlGuiWindow::saveNewToolbarConfig()
{
    // createGUI(xmlFile()); // this loses any plugged-in guiclients, so we use remove+add instead.

    guiFactory()->removeClient(this);
    guiFactory()->addClient(this);

    KConfigGroup cg(KSharedConfig::openConfig(), "");
    applyMainWindowSettings(cg);
}

void KXmlGuiWindow::setupGUI(StandardWindowOptions options, const QString &xmlfile)
{
    setupGUI(QSize(), options, xmlfile);
}

void KXmlGuiWindow::setupGUI(const QSize &defaultSize, StandardWindowOptions options, const QString &xmlfile)
{
    K_D(KXmlGuiWindow);

    if (options & Keys) {
        KStandardAction::keyBindings(guiFactory(),
                                     SLOT(configureShortcuts()), actionCollection());
    }

    if ((options & StatusBar) && statusBar()) {
        createStandardStatusBarAction();
    }

    if (options & ToolBar) {
        setStandardToolBarMenuEnabled(true);
        KStandardAction::configureToolbars(this,
                                           SLOT(configureToolbars()), actionCollection());
    }

    d->defaultSize = defaultSize;

    if (options & Create) {
        createGUI(xmlfile);
    }

    if (d->defaultSize.isValid()) {
        resize(d->defaultSize);
    } else if (isHidden()) {
        adjustSize();
    }

    if (options & Save) {
        const KConfigGroup cg(autoSaveConfigGroup());
        if (cg.isValid()) {
            setAutoSaveSettings(cg);
        } else {
            setAutoSaveSettings();
        }
    }
}
void KXmlGuiWindow::createGUI(const QString &xmlfile)
{
    K_D(KXmlGuiWindow);
    // disabling the updates prevents unnecessary redraws
    //setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient(this);

    // make sure to have an empty GUI
    QMenuBar *mb = menuBar();
    if (mb) {
        mb->clear();
    }

    qDeleteAll(toolBars());   // delete all toolbars

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        delete d->helpMenu;
        // we always want a help menu
        d->helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);

        KActionCollection *actions = actionCollection();
        QAction *helpContentsAction = d->helpMenu->action(KHelpMenu::menuHelpContents);
        QAction *whatsThisAction = d->helpMenu->action(KHelpMenu::menuWhatsThis);
        QAction *reportBugAction = d->helpMenu->action(KHelpMenu::menuReportBug);
        QAction *switchLanguageAction = d->helpMenu->action(KHelpMenu::menuSwitchLanguage);
        QAction *aboutAppAction = d->helpMenu->action(KHelpMenu::menuAboutApp);
        QAction *aboutKdeAction = d->helpMenu->action(KHelpMenu::menuAboutKDE);
        QAction *donateAction = d->helpMenu->action(KHelpMenu::menuDonate);

        if (helpContentsAction) {
            actions->addAction(helpContentsAction->objectName(), helpContentsAction);
        }
        if (whatsThisAction) {
            actions->addAction(whatsThisAction->objectName(), whatsThisAction);
        }
        if (reportBugAction) {
            actions->addAction(reportBugAction->objectName(), reportBugAction);
        }
        if (switchLanguageAction) {
            actions->addAction(switchLanguageAction->objectName(), switchLanguageAction);
        }
        if (aboutAppAction) {
            actions->addAction(aboutAppAction->objectName(), aboutAppAction);
        }
        if (aboutKdeAction) {
            actions->addAction(aboutKdeAction->objectName(), aboutKdeAction);
        }
        if (donateAction) {
            actions->addAction(donateAction->objectName(), donateAction);
        }
    }

    const QString windowXmlFile = xmlfile.isNull() ? componentName() + QLatin1String("ui.rc") : xmlfile;

    // Help beginners who call setXMLFile and then setupGUI...
    if (!xmlFile().isEmpty() && xmlFile() != windowXmlFile) {
        qCWarning(DEBUG_KXMLGUI) << "You called setXMLFile(" << xmlFile() << ") and then createGUI or setupGUI,"
                   << "which also calls setXMLFile and will overwrite the file you have previously set.\n"
                   << "You should call createGUI(" << xmlFile() << ") or setupGUI(<options>," << xmlFile() << ") instead.";
    }

    // we always want to load in our global standards file
    loadStandardsXmlFile();

    // now, merge in our local xml file.
    setXMLFile(windowXmlFile, true);

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument(QDomDocument());

    // do the actual GUI building
    guiFactory()->reset();
    guiFactory()->addClient(this);

    checkAmbiguousShortcuts();

    //  setUpdatesEnabled( true );
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate)
{
    stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate,
                                     bool reverse)
{
    stateChanged(newstate,
                 reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::setStandardToolBarMenuEnabled(bool enable)
{
    K_D(KXmlGuiWindow);
    if (enable) {
        if (d->toolBarHandler) {
            return;
        }

        d->toolBarHandler = new KDEPrivate::ToolBarHandler(this);

        if (factory()) {
            factory()->addClient(d->toolBarHandler);
        }
    } else {
        if (!d->toolBarHandler) {
            return;
        }

        if (factory()) {
            factory()->removeClient(d->toolBarHandler);
        }

        delete d->toolBarHandler;
        d->toolBarHandler = nullptr;
    }
}

bool KXmlGuiWindow::isStandardToolBarMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return (d->toolBarHandler);
}

void KXmlGuiWindow::createStandardStatusBarAction()
{
    K_D(KXmlGuiWindow);
    if (!d->showStatusBarAction) {
        d->showStatusBarAction = KStandardAction::showStatusbar(this, &KMainWindow::setSettingsDirty, actionCollection());
        QStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
        connect(d->showStatusBarAction, &QAction::toggled,
                sb, &QWidget::setVisible);
        d->showStatusBarAction->setChecked(sb->isHidden());
    } else {
        // If the language has changed, we'll need to grab the new text and whatsThis
        QAction *tmpStatusBar = KStandardAction::showStatusbar(nullptr, nullptr, nullptr);
        d->showStatusBarAction->setText(tmpStatusBar->text());
        d->showStatusBarAction->setWhatsThis(tmpStatusBar->whatsThis());
        delete tmpStatusBar;
    }
}

void KXmlGuiWindow::finalizeGUI(bool /*force*/)
{
    // FIXME: this really needs to be removed with a code more like the one we had on KDE3.
    //        what we need to do here is to position correctly toolbars so they don't overlap.
    //        Also, take in count plugins could provide their own toolbars and those also need to
    //        be restored.
    if (autoSaveSettings() && autoSaveConfigGroup().isValid()) {
        applyMainWindowSettings(autoSaveConfigGroup());
    }
}

void KXmlGuiWindow::applyMainWindowSettings(const KConfigGroup &config)
{
    K_D(KXmlGuiWindow);
    KMainWindow::applyMainWindowSettings(config);
    QStatusBar *sb = findChild<QStatusBar *>();
    if (sb && d->showStatusBarAction) {
        d->showStatusBarAction->setChecked(!sb->isHidden());
    }
}

// TODO KF6: change it to "using KXMLGUIBuilder::finalizeGUI;" in the header
// and remove the reimplementation
void KXmlGuiWindow::finalizeGUI(KXMLGUIClient *client)
{
    KXMLGUIBuilder::finalizeGUI(client);
}

void KXmlGuiWindow::checkAmbiguousShortcuts()
{
    QMap<QString, QAction*> shortcuts;
    QAction *editCutAction = actionCollection()->action(QStringLiteral("edit_cut"));
    QAction *deleteFileAction = actionCollection()->action(QStringLiteral("deletefile"));
    const auto actions = actionCollection()->actions();
    for (QAction *action : actions) {
        if (action->isEnabled()) {
            const auto actionShortcuts = action->shortcuts();
            for (const QKeySequence &shortcut : actionShortcuts) {
                if (shortcut.isEmpty()) {
                    continue;
                }
                const QString portableShortcutText = shortcut.toString();
                const QAction *existingShortcutAction = shortcuts.value(portableShortcutText);
                if (existingShortcutAction) {
                    // If the shortcut is already in use we give a warning, so that hopefully the developer will find it
                    // There is one exception, if the conflicting shortcut is a non primary shortcut of "edit_cut"
                    // and "deleteFileAction" is the other action since Shift+Delete is used for both in our default code
                    bool showWarning = true;
                    if ((action == editCutAction && existingShortcutAction == deleteFileAction) ||
                        (action == deleteFileAction && existingShortcutAction == editCutAction)) {
                        QList<QKeySequence> editCutActionShortcuts = editCutAction->shortcuts();
                        if (editCutActionShortcuts.indexOf(shortcut) > 0) // alternate shortcut
                        {
                            editCutActionShortcuts.removeAll(shortcut);
                            editCutAction->setShortcuts(editCutActionShortcuts);

                            showWarning = false;
                        }
                    }

                    if (showWarning) {
                        const QString actionName = KLocalizedString::removeAcceleratorMarker(action->text());
                        const QString existingShortcutActionName = KLocalizedString::removeAcceleratorMarker(existingShortcutAction->text());
                        QString dontShowAgainString = existingShortcutActionName + actionName + shortcut.toString();
                        dontShowAgainString.remove(QLatin1Char('\\'));
                        KMessageBox::information(this, i18n("There are two actions (%1, %2) that want to use the same shortcut (%3). This is most probably a bug. Please report it in <a href='https://bugs.kde.org'>bugs.kde.org</a>", existingShortcutActionName, actionName, shortcut.toString(QKeySequence::NativeText)), i18n("Ambiguous Shortcuts"), dontShowAgainString, KMessageBox::Notify | KMessageBox::AllowLink);
                    }
                } else {
                    shortcuts.insert(portableShortcutText, action);
                }
            }
        }
    }
}

#include "moc_kxmlguiwindow.cpp"

