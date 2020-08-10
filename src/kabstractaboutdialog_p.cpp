/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>
    SPDX-FileCopyrightText: 2008, 2019 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    Parts of this class have been take from the KAboutApplication class, which was
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kabstractaboutdialog_p.h"

#include "kaboutapplicationpersonmodel_p.h"
#include "kaboutapplicationpersonlistview_p.h"
#include "kaboutapplicationpersonlistdelegate_p.h"
#include "klicensedialog_p.h"
// KF
#include <KTitleWidget>
#include <KLocalizedString>
// Qt
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

QWidget *KAbstractAboutDialogPrivate::createTitleWidget(const QIcon &icon,
                                                        const QString &displayName,
                                                        const QString &version,
                                                        QWidget *parent)
{
    KTitleWidget *titleWidget = new KTitleWidget(parent);

    titleWidget->setIconSize(QSize(48, 48));
    titleWidget->setIcon(icon, KTitleWidget::ImageLeft);
    titleWidget->setText(i18n("<html><font size=\"5\">%1</font><br />Version %2</html>", displayName, version));

    return titleWidget;
}

QWidget *KAbstractAboutDialogPrivate::createAboutWidget(const QString &shortDescription,
                                                        const QString &otherText,
                                                        const QString &copyrightStatement,
                                                        const QString &homepage,
                                                        const QList<KAboutLicense> &licenses,
                                                        QWidget *parent)
{
    QWidget *aboutWidget = new QWidget(parent);
    QVBoxLayout *aboutLayout = new QVBoxLayout(aboutWidget);

    QString aboutPageText = shortDescription + QLatin1Char('\n');

    if (!otherText.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + otherText + QLatin1Char('\n');
    }

    if (!copyrightStatement.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + copyrightStatement + QLatin1Char('\n');
    }

    if (!homepage.isEmpty()) {
        aboutPageText += QLatin1Char('\n') + QStringLiteral("<a href=\"%1\">%1</a>").arg(homepage) + QLatin1Char('\n');
    }
    aboutPageText = aboutPageText.trimmed();

    QLabel *aboutLabel = new QLabel;
    aboutLabel->setWordWrap(true);
    aboutLabel->setOpenExternalLinks(true);
    aboutLabel->setText(aboutPageText.replace(QLatin1Char('\n'), QStringLiteral("<br />")));
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    aboutLayout->addStretch();
    aboutLayout->addWidget(aboutLabel);

    const int licenseCount = licenses.count();
    for (int i = 0; i < licenseCount; ++i) {
        const KAboutLicense &license = licenses.at(i);

        QLabel *showLicenseLabel = new QLabel;
        showLicenseLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>").arg(QString::number(i),
                                  i18n("License: %1", license.name(KAboutLicense::FullName))));
        showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        QObject::connect(showLicenseLabel, &QLabel::linkActivated, parent, [license, parent]() {
            auto *dialog = new KLicenseDialog(license, parent);
            dialog->show();
        });

        aboutLayout->addWidget(showLicenseLabel);
    }

    aboutLayout->addStretch();

    return aboutWidget;
}

QWidget *KAbstractAboutDialogPrivate::createAuthorsWidget(const QList<KAboutPerson> &authors,
                                                          const QString &ocsProviderUrl,
                                                          bool customAuthorTextEnabled,
                                                          const QString &customAuthorRichText,
                                                          const QString &bugAddress,
                                                          QWidget *parent)
{
    QWidget *authorWidget = new QWidget(parent);
    QVBoxLayout *authorLayout = new QVBoxLayout(authorWidget);
    authorLayout->setContentsMargins(0, 0, 0, 0);

    if (!customAuthorTextEnabled || !customAuthorRichText.isEmpty()) {
        QLabel *bugsLabel = new QLabel(authorWidget);
        bugsLabel->setContentsMargins(4, 2, 0, 4);
        bugsLabel->setOpenExternalLinks(true);
        if (!customAuthorTextEnabled) {
            if (bugAddress.isEmpty() || bugAddress == QLatin1String("submit@bugs.kde.org")) {
                bugsLabel->setText(i18n("Please use <a href=\"https://bugs.kde.org\">https://bugs.kde.org</a> to report bugs.\n"));
            } else {
                QUrl bugUrl(bugAddress);
                if (bugUrl.scheme().isEmpty()) {
                    bugUrl.setScheme(QStringLiteral("mailto"));
                }
                bugsLabel->setText(i18n("Please report bugs to <a href=\"%1\">%2</a>.\n",
                                        bugUrl.toString(), bugAddress));
            }
        } else {
            bugsLabel->setText(customAuthorRichText);
        }
        bugsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        authorLayout->addWidget(bugsLabel);
    }

    KDEPrivate::KAboutApplicationPersonModel *authorModel =
        new KDEPrivate::KAboutApplicationPersonModel(authors, ocsProviderUrl, authorWidget);

    KDEPrivate::KAboutApplicationPersonListView *authorView =
        new KDEPrivate::KAboutApplicationPersonListView(authorWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *authorDelegate =
        new KDEPrivate::KAboutApplicationPersonListDelegate(authorView, authorView);

    authorView->setModel(authorModel);
    authorView->setItemDelegate(authorDelegate);
    authorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    authorLayout->addWidget(authorView);

    return authorWidget;
}

QWidget *KAbstractAboutDialogPrivate::createCreditWidget(const QList<KAboutPerson> &credits,
                                                         const QString &ocsProviderUrl,
                                                         QWidget *parent)
{
    QWidget *creditWidget = new QWidget(parent);
    QVBoxLayout *creditLayout = new QVBoxLayout(creditWidget);
    creditLayout->setContentsMargins(0, 0, 0, 0);

    KDEPrivate::KAboutApplicationPersonModel *creditModel =
        new KDEPrivate::KAboutApplicationPersonModel(credits, ocsProviderUrl, creditWidget);

    KDEPrivate::KAboutApplicationPersonListView *creditView =
        new KDEPrivate::KAboutApplicationPersonListView(creditWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *creditDelegate =
        new KDEPrivate::KAboutApplicationPersonListDelegate(creditView, creditView);

    creditView->setModel(creditModel);
    creditView->setItemDelegate(creditDelegate);
    creditView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    creditLayout->addWidget(creditView);

    return creditWidget;
}

QWidget *KAbstractAboutDialogPrivate::createTranslatorsWidget(const QList<KAboutPerson> &translators,
                                                              const QString &ocsProviderUrl,
                                                              QWidget *parent)
{
    QWidget *translatorWidget = new QWidget(parent);
    QVBoxLayout *translatorLayout = new QVBoxLayout(translatorWidget);
    translatorLayout->setContentsMargins(0, 0, 0, 0);

    KDEPrivate::KAboutApplicationPersonModel *translatorModel =
        new KDEPrivate::KAboutApplicationPersonModel(translators, ocsProviderUrl, translatorWidget);

    KDEPrivate::KAboutApplicationPersonListView *translatorView =
        new KDEPrivate::KAboutApplicationPersonListView(translatorWidget);

    KDEPrivate::KAboutApplicationPersonListDelegate *translatorDelegate =
        new KDEPrivate::KAboutApplicationPersonListDelegate(translatorView, translatorView);

    translatorView->setModel(translatorModel);
    translatorView->setItemDelegate(translatorDelegate);
    translatorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    translatorLayout->addWidget(translatorView);

    QString aboutTranslationTeam = KAboutData::aboutTranslationTeam();
    if (!aboutTranslationTeam.isEmpty()) {
        QLabel *translationTeamLabel = new QLabel(translatorWidget);
        translationTeamLabel->setContentsMargins(4, 2, 4, 4);
        translationTeamLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        translationTeamLabel->setWordWrap(true);
        translationTeamLabel->setText(aboutTranslationTeam);
        translationTeamLabel->setOpenExternalLinks(true);
        translatorLayout->addWidget(translationTeamLabel);
        //TODO: this could be displayed as a view item to save space
    }

    return translatorWidget;
}

void KAbstractAboutDialogPrivate::createForm(QWidget *titleWidget, QWidget *tabWidget,
                                             QDialog *dialog)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    //And we jam everything together in a layout...
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
}
