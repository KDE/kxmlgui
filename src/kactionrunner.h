/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KACTIONRUNNER_H
#define KACTIONRUNNER_H

#include "kcombobox.h"
#include <kcompletion_export.h>

class KActionCollection;
class KActionRunnerPrivate;

class KCOMPLETION_EXPORT KActionRunner : public KComboBox {
    Q_OBJECT
public:
    KActionRunner(KActionCollection* ac, QWidget* parent);
    virtual ~KActionRunner();
public Q_SLOTS:
    void activate(int index);
private:
    KActionRunnerPrivate *d_ptr;
};

#endif
