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

#include "klicensedialog_p.h"
#include <kxmlgui_version.h>
// KF
#include <KAdjustingScrollArea>
#include <KLocalizedString>
#include <KSeparator>
#include <KTitleWidget>
// Qt
#include <QApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

QWidget *KAbstractAboutDialogPrivate::createTitleWidget(const QIcon &icon, const QString &displayName, const QString &version, QWidget *parent)
{
    KTitleWidget *titleWidget = new KTitleWidget(parent);

    titleWidget->setIconSize(QSize(48, 48));
    titleWidget->setIcon(icon, KTitleWidget::ImageLeft);
    titleWidget->setText(
        QLatin1String("<html><font size=\"5\">%1</font><br />%2</html>").arg(displayName, i18nc("Version version-number", "Version %1", version)));
    return titleWidget;
}

QWidget *KAbstractAboutDialogPrivate::createAboutWidget(const QString &shortDescription,
                                                        const QString &otherText,
                                                        const QString &copyrightStatement,
                                                        const QString &homepage,
                                                        const QList<KAboutLicense> &licenses,
                                                        QWidget *parent)
{
    auto wrapper = new KAdjustingScrollArea(parent);
    auto aboutWidget = new QWidget(parent);
    wrapper->setWidget(aboutWidget);
    auto aboutLayout = new QVBoxLayout(aboutWidget);

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

    aboutLayout->addWidget(aboutLabel);

    const int licenseCount = licenses.count();
    for (int i = 0; i < licenseCount; ++i) {
        const KAboutLicense &license = licenses.at(i);

        QLabel *showLicenseLabel = new QLabel;
        showLicenseLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>").arg(QString::number(i), i18n("License: %1", license.name(KAboutLicense::FullName))));
        showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
        QObject::connect(showLicenseLabel, &QLabel::linkActivated, parent, [license, parent]() {
            auto *dialog = new KLicenseDialog(license, parent);
            dialog->show();
        });

        aboutLayout->addWidget(showLicenseLabel);
    }

    aboutLayout->addStretch();

    return wrapper;
}

QWidget *KAbstractAboutDialogPrivate::createComponentWidget(const QList<KAboutComponent> &components, QWidget *parent)
{
    auto wrapper = new KAdjustingScrollArea(parent);
    auto componentWidget = new QWidget;
    auto componentLayout = new QVBoxLayout(componentWidget);
    wrapper->setWidget(componentWidget);

    QList<KAboutComponent> allComponents = components;
    allComponents.prepend(KAboutComponent(i18n("The <em>%1</em> windowing system", QGuiApplication::platformName())));
    allComponents.prepend(KAboutComponent(i18n("Qt"),
                                          QString(),
                                          i18n("%1 (built against %2)", QString::fromLocal8Bit(qVersion()), QStringLiteral(QT_VERSION_STR)),
                                          QStringLiteral("https://www.qt.io/")));
    allComponents.prepend(KAboutComponent(i18n("KDE Frameworks"),
                                          QString(),
                                          QStringLiteral(KXMLGUI_VERSION_STRING),
                                          QStringLiteral("https://develop.kde.org/products/frameworks/")));

    for (qsizetype i = 0, count = allComponents.count(); i < count; i++) {
        const auto &component = allComponents[i];
        QVBoxLayout *col = nullptr;
        QHBoxLayout *row = nullptr;
        auto name = new QLabel(u"<span style='font-weight: 600'>"_s + component.name() + u"</span>"_s
                               + (!component.version().isEmpty() ? (u" (" + component.version() + u')') : QString{}));
        if (!component.description().isEmpty()) {
            col = new QVBoxLayout;
            col->setSpacing(0);
            auto description = new QLabel(component.description());
            col->addWidget(name);
            col->addWidget(description);
        }

        if (!component.webAddress().isEmpty()) {
            row = new QHBoxLayout;
            const auto url = QUrl(component.webAddress());
            auto webAction = new QAction(QIcon::fromTheme(u"internet-services-symbolic"_s), i18nc("@action:button", "Visit component's homepage"));
            webAction->setToolTip(i18nc("@info:tooltip", "Visit components's homepage\n%1", component.webAddress()));
            QObject::connect(webAction, &QAction::triggered, webAction, [url]() {
                QDesktopServices::openUrl(QUrl(url));
            });
            auto web = new QToolButton;
            web->setDefaultAction(webAction);
            web->setToolButtonStyle(Qt::ToolButtonIconOnly);
            web->setAutoRaise(true);
            if (col) {
                row->addLayout(col);
            } else {
                row->addWidget(name);
            }
            row->addWidget(web);
        }

        if (row) {
            componentLayout->addLayout(row);
        } else if (col) {
            componentLayout->addLayout(col);
        } else {
            componentLayout->addWidget(name);
        }

        if (i + 1 != count) {
            auto separator = new KSeparator;
            separator->setEnabled(false);
            componentLayout->addWidget(separator);
        }
    }

    componentLayout->addStretch();

    return wrapper;
}

static void createPersonLayout(QVBoxLayout *layout, const QList<KAboutPerson> &persons)
{
    for (qsizetype i = 0, count = persons.count(); i < count; i++) {
        const auto &person = persons[i];

        QVBoxLayout *col = nullptr;
        QHBoxLayout *row = nullptr;
        auto name = new QLabel(person.name());
        auto font = name->font();
        font.setWeight(QFont::DemiBold);
        name->setFont(font);
        if (!person.task().isEmpty()) {
            col = new QVBoxLayout;
            col->setSpacing(0);
            auto task = new QLabel(person.task());
            auto palette = task->palette();
            auto foregroundColor = palette.color(QPalette::WindowText);
            foregroundColor.setAlphaF(0.85);
            palette.setColor(QPalette::WindowText, foregroundColor);
            task->setPalette(palette);

            col->addWidget(name);
            col->addWidget(task);
        }

        if (!person.webAddress().isEmpty()) {
            row = new QHBoxLayout;
            const auto url = QUrl(person.webAddress());
            auto webAction = new QAction(QIcon::fromTheme(u"internet-services-symbolic"_s), i18nc("@action:button", "Visit author's homepage"));
            webAction->setToolTip(i18nc("@info:tooltip", "Visit author's homepage\n%1", person.webAddress()));
            QObject::connect(webAction, &QAction::triggered, webAction, [url]() {
                QDesktopServices::openUrl(url);
            });
            auto web = new QToolButton;
            web->setDefaultAction(webAction);
            web->setAutoRaise(true);
            web->setToolButtonStyle(Qt::ToolButtonIconOnly);
            if (col) {
                row->addLayout(col);
            } else {
                row->addWidget(name);
            }
            row->addWidget(web);
        }

        if (!person.emailAddress().isEmpty()) {
            if (!row) {
                row = new QHBoxLayout;
            }
            const auto url = person.emailAddress();
            auto webAction =
                new QAction(QIcon::fromTheme(u"mail-send-symbolic"_s), i18nc("@action:button Send an email to a contributor", "Email contributor"));
            webAction->setToolTip(i18nc("@info:tooltip", "Email contributor: %1", person.emailAddress()));
            QObject::connect(webAction, &QAction::triggered, webAction, [url]() {
                QDesktopServices::openUrl(QUrl(u"mailto:"_s + url));
            });
            auto web = new QToolButton;
            web->setDefaultAction(webAction);
            web->setToolButtonStyle(Qt::ToolButtonIconOnly);
            web->setAutoRaise(true);
            if (col) {
                row->addLayout(col);
            } else {
                row->addWidget(name);
            }
            row->addWidget(web);
        }

        if (row) {
            layout->addLayout(row);
        } else if (col) {
            layout->addLayout(col);
        } else {
            layout->addWidget(name);
        }

        if (i + 1 != count) {
            auto separator = new KSeparator;
            separator->setEnabled(false);
            layout->addWidget(separator);
        }
    }

    layout->addStretch();
}

QWidget *KAbstractAboutDialogPrivate::createAuthorsWidget(const QList<KAboutPerson> &authors,
                                                          bool customAuthorTextEnabled,
                                                          const QString &customAuthorRichText,
                                                          const QString &bugAddress,
                                                          QWidget *parent)
{
    auto wrapper = new KAdjustingScrollArea;
    auto authorWidget = new QWidget(parent);
    wrapper->setWidget(authorWidget);
    auto authorLayout = new QVBoxLayout(authorWidget);

    if (!customAuthorTextEnabled || !customAuthorRichText.isEmpty()) {
        QLabel *bugsLabel = new QLabel(authorWidget);
        bugsLabel->setOpenExternalLinks(true);
        if (!customAuthorTextEnabled) {
            if (bugAddress.isEmpty() || bugAddress == QLatin1String("submit@bugs.kde.org")) {
                bugsLabel->setText(i18nc("Reference to website",
                                         "Please use %1 to report bugs.\n",
                                         QLatin1String("<a href=\"https://bugs.kde.org\">https://bugs.kde.org</a>")));
            } else {
                QUrl bugUrl(bugAddress);
                if (bugUrl.scheme().isEmpty()) {
                    bugUrl.setScheme(QStringLiteral("mailto"));
                }
                bugsLabel->setText(i18nc("Reference to email address",
                                         "Please report bugs to %1.\n",
                                         QLatin1String("<a href=\"%1\">%2</a>").arg(bugUrl.toString(), bugAddress)));
            }
        } else {
            bugsLabel->setText(customAuthorRichText);
        }
        bugsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        authorLayout->addWidget(bugsLabel);
    }

    createPersonLayout(authorLayout, authors);

    return wrapper;
}

QWidget *KAbstractAboutDialogPrivate::createCreditWidget(const QList<KAboutPerson> &credits, QWidget *parent)
{
    auto wrapper = new KAdjustingScrollArea;
    auto creditWidget = new QWidget(parent);
    wrapper->setWidget(creditWidget);
    auto creditLayout = new QVBoxLayout(creditWidget);

    createPersonLayout(creditLayout, credits);

    return wrapper;
}

QWidget *KAbstractAboutDialogPrivate::createTranslatorsWidget(const QList<KAboutPerson> &translators, QWidget *parent)
{
    auto wrapper = new KAdjustingScrollArea;
    auto translatorWidget = new QWidget(parent);
    wrapper->setWidget(translatorWidget);
    auto translatorLayout = new QVBoxLayout(translatorWidget);

    createPersonLayout(translatorLayout, translators);

    QString aboutTranslationTeam = KAboutData::aboutTranslationTeam();
    if (!aboutTranslationTeam.isEmpty()) {
        QLabel *translationTeamLabel = new QLabel(translatorWidget);
        translationTeamLabel->setContentsMargins(4, 2, 4, 4);
        translationTeamLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        translationTeamLabel->setWordWrap(true);
        translationTeamLabel->setText(aboutTranslationTeam);
        translationTeamLabel->setOpenExternalLinks(true);
        translatorLayout->addWidget(translationTeamLabel);
        // TODO: this could be displayed as a view item to save space
    }

    return wrapper;
}

void KAbstractAboutDialogPrivate::createForm(QWidget *titleWidget, QWidget *tabWidget, QDialog *dialog)
{
    QDialogButtonBox *buttonBox = new QDialogButtonBox(dialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // And we jam everything together in a layout...
    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
}
