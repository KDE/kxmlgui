/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>
   Copyright (C) 2008, 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
   Copyright (C) 2010 Teo Mrnjavac <teo@kde.org>

   Parts of this class have been take from the KAboutApplication class, which was
   Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and Espen Sand (espen@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kaboutplugindialog.h"

#include "kabstractaboutdialog_p.h"
// KF
#include <KWidgetItemDelegate>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KAboutData>
// Qt
#include <QTabWidget>
#include <QGuiApplication>

class KAboutPluginDialogPrivate : public KAbstractAboutDialogPrivate
{
public:
    KAboutPluginDialogPrivate(const KPluginMetaData &pluginMetaData, KAboutPluginDialog *parent)
        : q(parent)
        , pluginMetaData(pluginMetaData)
        , pluginLicense(KAboutLicense::byKeyword(pluginMetaData.license()))
    {}

    void init(KAboutPluginDialog::Options opt);

public:
    KAboutPluginDialog * const q;

    const KPluginMetaData pluginMetaData;
    const KAboutLicense pluginLicense;
};


KAboutPluginDialog::KAboutPluginDialog(const KPluginMetaData &pluginMetaData, QWidget *parent)
    : KAboutPluginDialog(pluginMetaData, NoOptions, parent)
{
}

KAboutPluginDialog::KAboutPluginDialog(const KPluginMetaData &pluginMetaData, Options opt, QWidget *parent)
    : QDialog(parent)
    , d(new KAboutPluginDialogPrivate(pluginMetaData,this))
{
    d->init(opt);
}

KAboutPluginDialog::~KAboutPluginDialog()
{
    // The delegates want to be deleted before the items it created
    qDeleteAll(findChildren<KWidgetItemDelegate *>());
}

void KAboutPluginDialogPrivate::init(KAboutPluginDialog::Options opt)
{
    q->setWindowTitle(i18nc("@title:window", "About %1", pluginMetaData.name()));

    //Set up the title widget...
    const QIcon pluginIcon = !pluginMetaData.iconName().isEmpty() ? QIcon::fromTheme(pluginMetaData.iconName()) :
    qApp->windowIcon();
    QWidget *titleWidget = createTitleWidget(pluginIcon.pixmap(48, 48),
                                             pluginMetaData.name(), pluginMetaData.version(), q);

    //Then the tab bar...
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    //Set up the first page...
    QWidget *aboutWidget = createAboutWidget(pluginMetaData.description(), pluginMetaData.extraInformation(),
                                             pluginMetaData.copyrightText(), pluginMetaData.website(),
                                             {pluginLicense}, q);

    tabWidget->addTab(aboutWidget, i18nc("@title:tab", "About"));

    //And here we go, authors page...
    const int authorCount = pluginMetaData.authors().count();
    if (authorCount) {
        // TODO: add bug report address to plugin metadata
        QWidget *authorWidget = createAuthorsWidget(pluginMetaData.authors(), QString(),
                                                    false,
                                                    QString(),
                                                    QString(), q);

        const QString authorPageTitle = i18ncp("@title:tab", "Author", "Authors", authorCount);
        tabWidget->addTab(authorWidget, authorPageTitle);
    }

    //And credits page...
    if (!pluginMetaData.otherContributors().isEmpty()) {
        QWidget *creditWidget = createCreditWidget(pluginMetaData.otherContributors(), QString(), q);
        tabWidget->addTab(creditWidget, i18nc("@title:tab", "Thanks To"));
    }

    //Finally, the optional translators page...
    if (!(opt & KAboutPluginDialog::HideTranslators) && !pluginMetaData.translators().isEmpty()) {
        QWidget *translatorWidget = createTranslatorsWidget(pluginMetaData.translators(), QString(), q);
        tabWidget->addTab(translatorWidget, i18nc("@title:tab", "Translation"));
    }

    createForm(titleWidget, tabWidget, q);
}
