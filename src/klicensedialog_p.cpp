/* This file is part of the KDE libraries
   Copyright (C) 2007 Urs Wolfer <uwolfer at kde.org>
   Copyright (C) 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "klicensedialog_p.h"

// KF
#include <KLocalizedString>
#include <KAboutData>
// Qt
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QTextBrowser>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QStyle>

KLicenseDialog::KLicenseDialog(const KAboutLicense &license, QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    setWindowTitle(i18nc("@title:window", "License Agreement"));

    const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    const QString licenseText = license.text();

    QTextBrowser *licenseBrowser = new QTextBrowser(this);
    licenseBrowser->setFont(font);
    licenseBrowser->setLineWrapMode(QTextEdit::NoWrap);
    licenseBrowser->setText(licenseText);
    layout->addWidget(licenseBrowser);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    // try to set up the dialog such that the full width of the
    // document is visible without horizontal scroll-bars being required
    const int marginHint = style()->pixelMetric(QStyle::PM_DefaultChildMargin);
    const qreal idealWidth = licenseBrowser->document()->idealWidth() + (2 * marginHint)
                             + licenseBrowser->verticalScrollBar()->width() * 2;

    // try to allow enough height for a reasonable number of lines to be shown
    QFontMetrics metrics(font);
    const int idealHeight = metrics.height() * 30;

    resize(sizeHint().expandedTo(QSize(qRound(idealWidth), idealHeight)));
}

KLicenseDialog::~KLicenseDialog() = default;
