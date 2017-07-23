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

#include "kactionrunnermodel.h"
#include <KActionCollection>
#include <QDebug>
#include <QApplication>

KActionRunnerModel::KActionRunnerModel(KActionCollection* ac) : m_actionCollection(ac), m_size(0)
{
    m_size = ac->actions().size();
    connect(ac, &KActionCollection::inserted, [this](QAction *action){
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_size += 1;
        endInsertRows();
    });
}

KActionRunnerModel::~KActionRunnerModel()
{
}

QVariant KActionRunnerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QAction *action = m_actionCollection->action(index.row());
    if (!action)
        return QVariant();

    switch(role) {
        case Qt::DisplayRole : return action->text().remove(QLatin1Char('&'));
        case Qt::ToolTipRole : return action->toolTip();
        case Qt::DecorationRole : return action->icon();
        case Qt::AccessibleTextRole : return action->text().remove(QLatin1Char('&'));
        case Qt::AccessibleDescriptionRole : return action->toolTip();
        case Qt::BackgroundColorRole : {
            auto p = qApp->palette();
            p.setCurrentColorGroup(QPalette::Disabled);
            return !action->isEnabled() ? p.color(QPalette::Base) : QVariant();
        }
        default:
            return QVariant();
    }
    return QVariant();
}

int KActionRunnerModel::rowCount(const QModelIndex& parent) const
{
    return m_size;
}

void KActionRunnerModel::activate(int index)
{
    if (index >= m_actionCollection->actions().size())
        return;
    auto action = m_actionCollection->actions().at(index);
    if (action->isEnabled()) {
        action->trigger();
    }
}

