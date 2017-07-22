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

#include "kactionrunner.h"
#include "kactionrunnermodel.h"
#include <KActionCollection>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QShowEvent>
#include <QLineEdit>

class KActionRunnerPrivate {
public:
    KActionRunnerPrivate(KActionRunner* q, KActionCollection *ac)
        : q_ptr(q), model(new KActionRunnerModel(ac)), actionCollection(ac), completion(q_ptr->completionObject())
    {
        q_ptr->setEditable(true);
    }

    KActionRunner *q_ptr;
    KActionRunnerModel* model;
    KActionCollection *actionCollection;
    KCompletion *completion;
    QSortFilterProxyModel *filterModel;
};

KActionRunner::KActionRunner(KActionCollection* ac, QWidget* parent) :
    KComboBox(true, parent),
    d_ptr(new KActionRunnerPrivate(this, ac))
{

    setModel(d_ptr->model);
    setCompletionMode(KCompletion::CompletionPopupAuto);
    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [this](int index){
        activate(index);
        hide();
    });
 }

 void KActionRunner::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
    KComboBox::keyPressEvent(event);
}

void KActionRunner::showEvent(QShowEvent* event)
{
    setFocus();
    KComboBox::showEvent(event);
}

KActionRunner::~KActionRunner()
{
    delete d_ptr;
}

void KActionRunner::activate(int index)
{
    if (!isVisible())
        return;
    d_ptr->model->activate(index);
}
