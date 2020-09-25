/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    Parts of this class have been take from the KAboutApplication class, which was:
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kaboutapplicationdialog.h"

#include "kabstractaboutdialog_p.h"
#include "../kxmlgui_version.h"
// KF
#include <KAboutData>
#include <KLocalizedString>
#include <KWidgetItemDelegate>
// Qt
#include <QApplication>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>


class Q_DECL_HIDDEN KAboutApplicationDialog::Private : public KAbstractAboutDialogPrivate
{
public:
    Private(const KAboutData &aboutData, KAboutApplicationDialog *parent)
        : q(parent)
        , aboutData(aboutData)
    {}

    void init(Options opt);

private:
    KAboutApplicationDialog * const q;

    const KAboutData aboutData;
};


KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData &aboutData, QWidget *parent)
    : KAboutApplicationDialog(aboutData, NoOptions, parent)
{
}

KAboutApplicationDialog::KAboutApplicationDialog(const KAboutData &aboutData, Options opt, QWidget *parent)
    : QDialog(parent)
    , d(new Private(aboutData, this))
{
    d->init(opt);
}

void KAboutApplicationDialog::Private::init(Options opt)
{
    q->setWindowTitle(i18nc("@title:window", "About %1", aboutData.displayName()));

    //Set up the title widget...
    QIcon titleIcon;
    if (aboutData.programLogo().canConvert<QPixmap>()) {
        titleIcon = QIcon(aboutData.programLogo().value<QPixmap>());
    } else if (aboutData.programLogo().canConvert<QImage>()) {
        titleIcon = QIcon(QPixmap::fromImage(aboutData.programLogo().value<QImage>()));
    } else if (aboutData.programLogo().canConvert<QIcon>()) {
        titleIcon = aboutData.programLogo().value<QIcon>();
    } else {
        titleIcon = qApp->windowIcon();
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 2)
        // Legacy support for deprecated KAboutData::programIconName()
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
        if (titleIcon.isNull() && !aboutData.programIconName().isEmpty()) {
            titleIcon = QIcon::fromTheme(aboutData.programIconName());
        }
QT_WARNING_POP
#endif
    }

    QWidget *titleWidget = createTitleWidget(titleIcon, aboutData.displayName(), aboutData.version(), q);

    //Then the tab bar...
    QTabWidget *tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(false);

    //Set up the first page...
    QWidget *aboutWidget = createAboutWidget(aboutData.shortDescription(), aboutData.otherText(),
                                             aboutData.copyrightStatement(), aboutData.homepage(),
                                             aboutData.licenses(), q);

    tabWidget->addTab(aboutWidget, i18nc("@title:tab", "About"));

    // Library versions
    if (!(opt & HideKdeVersion)) {
        QWidget *versionWidget = new QWidget(q);
        QVBoxLayout *versionLayout = new QVBoxLayout;
        QLabel *versionLabel = new QLabel(
            i18n("<ul><li>KDE Frameworks %1</li><li>Qt %2 (built against %3)</li><li>The <em>%4</em> windowing system</li></ul>",
                 QStringLiteral(KXMLGUI_VERSION_STRING),
                 QString::fromLocal8Bit(qVersion()),
                 QStringLiteral(QT_VERSION_STR),
                 QGuiApplication::platformName()));
        versionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        versionLayout->addWidget(versionLabel);
        versionLayout->addStretch();
        versionWidget->setLayout(versionLayout);
        tabWidget->addTab(versionWidget, i18nc("@title:tab", "Libraries"));
    }

    //And here we go, authors page...
    const int authorCount = aboutData.authors().count();
    if (authorCount) {
        QWidget *authorWidget = createAuthorsWidget(aboutData.authors(), aboutData.ocsProviderUrl(),
                                                    aboutData.customAuthorTextEnabled(),
                                                    aboutData.customAuthorRichText(),
                                                    aboutData.bugAddress(), q);

        const QString authorPageTitle = i18ncp("@title:tab", "Author", "Authors", authorCount);
        tabWidget->addTab(authorWidget, authorPageTitle);
    }

    //And credits page...
    if (!aboutData.credits().isEmpty()) {
        QWidget *creditWidget = createCreditWidget(aboutData.credits(), aboutData.ocsProviderUrl(), q);
        tabWidget->addTab(creditWidget, i18nc("@title:tab", "Thanks To"));
    }

    //Finally, the optional translators page...
    if (!(opt & HideTranslators) && !aboutData.translators().isEmpty()) {
        QWidget *translatorWidget = createTranslatorsWidget(aboutData.translators(), aboutData.ocsProviderUrl(), q);

        tabWidget->addTab(translatorWidget, i18nc("@title:tab", "Translation"));
    }

    createForm(titleWidget, tabWidget, q);
}

KAboutApplicationDialog::~KAboutApplicationDialog()
{
    delete d;
    // The delegate wants to be deleted before the items it created, otherwise
    // complains bitterly about it
    qDeleteAll(findChildren<KWidgetItemDelegate *>());
}
