/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>
    SPDX-FileCopyrightText: 2008 Alexander Dymo <adymo@kdevelop.org>
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshortcutsdialog.h"
#include "kshortcutsdialog_p.h"
#include "kshortcutschemeshelper_p.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QDomDocument>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>
#include <ksharedconfig.h>

#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kactioncollection.h"

/************************************************************************/
/* KShortcutsDialog                                                     */
/*                                                                      */
/* Originally by Nicolas Hadacek <hadacek@via.ecp.fr>                   */
/*                                                                      */
/* Substantially revised by Mark Donohoe <donohoe@kde.org>              */
/*                                                                      */
/* And by Espen Sand <espen@kde.org> 1999-10-19                         */
/* (by using KDialog there is almost no code left ;)                    */
/*                                                                      */
/************************************************************************/

QKeySequence primarySequence(const QList<QKeySequence> &sequences)
{
    return sequences.isEmpty() ? QKeySequence() : sequences.at(0);
}

QKeySequence alternateSequence(const QList<QKeySequence> &sequences)
{
    return sequences.size() <= 1 ? QKeySequence() : sequences.at(1);
}

class Q_DECL_HIDDEN KShortcutsDialog::KShortcutsDialogPrivate
{
public:

    KShortcutsDialogPrivate(KShortcutsDialog *q)
        : q(q)
    {
    }

    QList<KActionCollection *> m_collections;

    void changeShortcutScheme(const QString &scheme)
    {
        if (m_keyChooser->isModified() && KMessageBox::questionYesNo(q,
                i18n("The current shortcut scheme is modified. Save before switching to the new one?")) == KMessageBox::Yes) {
            m_keyChooser->save();
        } else {
            m_keyChooser->undo();
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_keyChooser->clearCollections();

        for (KActionCollection *collection : qAsConst(m_collections)) {
            // passing an empty stream forces the clients to reread the XML
            KXMLGUIClient *client = const_cast<KXMLGUIClient *>(collection->parentGUIClient());
            if (client) {
                client->setXMLGUIBuildDocument(QDomDocument());
            }
        }

        //get xmlguifactory
        if (!m_collections.isEmpty()) {
            const KXMLGUIClient *client = m_collections.first()->parentGUIClient();
            if (client) {
                KXMLGUIFactory *factory = client->factory();
                if (factory) {
                    factory->changeShortcutScheme(scheme);
                }
            }
        }

        for (KActionCollection *collection : qAsConst(m_collections)) {
            m_keyChooser->addCollection(collection);
        }

        QApplication::restoreOverrideCursor();
    }

    void undo()
    {
        m_keyChooser->undo();
    }

    void toggleDetails()
    {
        const bool isVisible = m_schemeEditor->isVisible();

        m_schemeEditor->setVisible(!isVisible);
        m_detailsButton->setText(detailsButtonText() + (isVisible ? QStringLiteral(" >>") : QStringLiteral(" <<")));
    }

    static QString detailsButtonText()
    {
        return i18n("Manage &Schemes");
    }

    void save()
    {
        m_keyChooser->save();
        emit q->saved();
    }

    KShortcutsDialog *const q;
    KShortcutsEditor *m_keyChooser = nullptr; // ### move
    KShortcutSchemesEditor *m_schemeEditor = nullptr;
    QPushButton *m_detailsButton = nullptr;
    bool m_saveSettings = false;
};

KShortcutsDialog::KShortcutsDialog(KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts, QWidget *parent)
    : QDialog(parent), d(new KShortcutsDialogPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Keyboard Shortcuts"));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    d->m_keyChooser = new KShortcutsEditor(this, types, allowLetterShortcuts);
    layout->addWidget(d->m_keyChooser);

    d->m_schemeEditor = new KShortcutSchemesEditor(this);
    connect(d->m_schemeEditor, SIGNAL(shortcutsSchemeChanged(QString)),
            this, SLOT(changeShortcutScheme(QString)));
    d->m_schemeEditor->hide();
    layout->addWidget(d->m_schemeEditor);

    d->m_detailsButton = new QPushButton;
    d->m_detailsButton->setText(KShortcutsDialogPrivate::detailsButtonText() + QLatin1String(" >>"));

    QPushButton *printButton = new QPushButton;
    KGuiItem::assign(printButton, KStandardGuiItem::print());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(d->m_detailsButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(printButton, QDialogButtonBox::ActionRole);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    layout->addWidget(buttonBox);

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked,
            d->m_keyChooser, &KShortcutsEditor::allDefault);
    connect(d->m_detailsButton, SIGNAL(clicked()), this, SLOT(toggleDetails()));
    connect(printButton, &QPushButton::clicked,
            d->m_keyChooser, &KShortcutsEditor::printShortcuts);
    connect(buttonBox, &QDialogButtonBox::rejected, this, [this]() { d->undo(); });

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    KConfigGroup group(KSharedConfig::openConfig(), "KShortcutsDialog Settings");
    resize(group.readEntry("Dialog Size", sizeHint()));
}

KShortcutsDialog::~KShortcutsDialog()
{
    KConfigGroup group(KSharedConfig::openConfig(), "KShortcutsDialog Settings");
    group.writeEntry("Dialog Size", size(), KConfigGroup::Persistent | KConfigGroup::Global);
    delete d;
}

void KShortcutsDialog::addCollection(KActionCollection *collection, const QString &title)
{
    d->m_keyChooser->addCollection(collection, title);
    d->m_collections << collection;
}

QList<KActionCollection *> KShortcutsDialog::actionCollections() const
{
    return d->m_collections;
}

//FIXME should there be a setSaveSettings method?
bool KShortcutsDialog::configure(bool saveSettings)
{
    d->m_saveSettings = saveSettings;
    if (isModal()) {
        int retcode = exec();
        return retcode;
    } else {
        show();
        return false;
    }
}

void KShortcutsDialog::accept()
{
    if (d->m_saveSettings) {
        d->save();
    }
    QDialog::accept();
}

QSize KShortcutsDialog::sizeHint() const
{
    return QSize(600, 480);
}

int KShortcutsDialog::configure(KActionCollection *collection, KShortcutsEditor::LetterShortcuts allowLetterShortcuts,
                                QWidget *parent, bool saveSettings)
{
    //qDebug(125) << "KShortcutsDialog::configureKeys( KActionCollection*, " << saveSettings << " )";
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, allowLetterShortcuts, parent);
    dlg.d->m_keyChooser->addCollection(collection);
    return dlg.configure(saveSettings);
}

void KShortcutsDialog::importConfiguration(const QString &path)
{
    KConfig config(path);
    d->m_keyChooser->importConfiguration(static_cast<KConfigBase *>(&config));
}

void KShortcutsDialog::exportConfiguration(const QString &path) const
{
    KConfig config(path);
    d->m_keyChooser->exportConfiguration(static_cast<KConfigBase *>(&config));
}

#include "moc_kshortcutsdialog.cpp"
