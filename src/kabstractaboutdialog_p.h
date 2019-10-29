/* This file is part of the KDE libraries
   Copyright 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KABSTRACTABOUTDIALOG_P_H
#define KABSTRACTABOUTDIALOG_P_H

#include <KAboutData>

class QDialog;
class QWidget;
class QPixmap;

/**
 * @internal
 *
 * Private base class implementing util methods for assembling an About dialog.
 */
class KAbstractAboutDialogPrivate
{
public:
    KAbstractAboutDialogPrivate() = default;
    ~KAbstractAboutDialogPrivate() = default;

public:
    QWidget *createTitleWidget(const QPixmap &pixmap,
                               const QString &displayName,
                               const QString &version,
                               QWidget *parent);
    QWidget *createAboutWidget(const QString &shortDescription,
                               const QString &otherText,
                               const QString &copyrightStatement,
                               const QString &homepage,
                               const QList<KAboutLicense> &licenses,
                               QWidget *parent);
    QWidget *createAuthorsWidget(const QList<KAboutPerson> &authors,
                                 const QString &ocsProviderUrl,
                                 bool customAuthorTextEnabled,
                                 const QString &customAuthorRichText,
                                 const QString &bugAddress,
                                 QWidget *parent);
    QWidget *createCreditWidget(const QList<KAboutPerson> &credits,
                                const QString &ocsProviderUrl,
                                QWidget *parent);
    QWidget *createTranslatorsWidget(const QList<KAboutPerson> &translators,
                                     const QString &ocsProviderUrl,
                                     QWidget *parent);
    void createForm(QWidget *titleWidget, QWidget *tabWidget,
                    QDialog *dialog);
};

#endif
