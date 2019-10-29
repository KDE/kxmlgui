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

#ifndef KLICENSEDIALOG_P_H
#define KLICENSEDIALOG_P_H

// Qt
#include <QDialog>

class KAboutLicense;

/**
 * @internal
 *
 * A dialog to display a license
 */
class KLicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KLicenseDialog(const KAboutLicense &license, QWidget *parent = nullptr);
    ~KLicenseDialog() override;

private:
    Q_DISABLE_COPY(KLicenseDialog)
};

#endif
