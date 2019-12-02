/* This file is part of the KDE libraries Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
    Copyright (C) 1997 Nicolas Hadacek <hadacek@kde.org>
    Copyright (C) 1998 Matthias Ettrich <ettrich@kde.org>
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>
    Copyright (C) 2007 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "config-xmlgui.h"

#include "kshortcutsdialog_p.h"
#include "debug.h"

#include <QAction>
#include <QTreeWidgetItem>

#if HAVE_GLOBALACCEL
# include <kglobalaccel.h>
#endif

KShortcutsEditorItem::KShortcutsEditorItem(QTreeWidgetItem *parent, QAction *action)
    : QTreeWidgetItem(parent, ActionItem)
    , m_action(action)
    , m_isNameBold(false)
    , m_oldLocalShortcut(nullptr)
    , m_oldGlobalShortcut(nullptr)
{
    // Filtering message requested by translators (scripting).
    m_id = m_action->objectName();
    m_actionNameInTable = i18nc("@item:intable Action name in shortcuts configuration", "%1", KLocalizedString::removeAcceleratorMarker(m_action->text()));
    if (m_actionNameInTable.isEmpty()) {
        qCWarning(DEBUG_KXMLGUI) << "Action without text!" << m_action->objectName();
        m_actionNameInTable = m_id;
    }

    m_collator.setNumericMode(true);
    m_collator.setCaseSensitivity(Qt::CaseSensitive);
}

KShortcutsEditorItem::~KShortcutsEditorItem()
{
    delete m_oldLocalShortcut;
    delete m_oldGlobalShortcut;
}

bool KShortcutsEditorItem::isModified() const
{
    return m_oldLocalShortcut || m_oldGlobalShortcut;
}

QVariant KShortcutsEditorItem::data(int column, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (column) {
        case Name:
            return m_actionNameInTable;
        case Id:
            return m_id;
        case LocalPrimary:
        case LocalAlternate:
        case GlobalPrimary:
        case GlobalAlternate:
            return keySequence(column);
        default:
            break;
        }
        break;
    case Qt::DecorationRole:
        if (column == Name) {
            return m_action->icon();
        } else {
            return QIcon();
        }
    case Qt::WhatsThisRole:
        return m_action->whatsThis();
    case Qt::ToolTipRole:
        // There is no such thing as a QAction::description(). So we have
        // nothing to display here.
        return QVariant();
    case Qt::FontRole:
        if (column == Name && m_isNameBold) {
            QFont modifiedFont = treeWidget()->font();
            modifiedFont.setBold(true);
            return modifiedFont;
        }
        break;
    case KExtendableItemDelegate::ShowExtensionIndicatorRole:
        switch (column) {
        case Name:
            return false;
        case LocalPrimary:
        case LocalAlternate:
            return !m_action->property("isShortcutConfigurable").isValid()
                   || m_action->property("isShortcutConfigurable").toBool();
#if HAVE_GLOBALACCEL
        case GlobalPrimary:
        case GlobalAlternate:
            if (!KGlobalAccel::self()->hasShortcut(m_action)) {
                return false;
            }
            return true;
#endif
        default:
            return false;
        }
    //the following are custom roles, defined in this source file only
    case ShortcutRole:
        switch (column) {
        case LocalPrimary:
        case LocalAlternate:
        case GlobalPrimary:
        case GlobalAlternate:
            return keySequence(column);
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }

    case DefaultShortcutRole: {
        QList<QKeySequence> defaultShortcuts = m_action->property("defaultShortcuts").value<QList<QKeySequence> >();
#if HAVE_GLOBALACCEL
        QList<QKeySequence> defaultGlobalShortcuts = KGlobalAccel::self()->defaultShortcut(m_action);
#endif

        switch (column) {
        case LocalPrimary:
            return primarySequence(defaultShortcuts);
        case LocalAlternate:
            return alternateSequence(defaultShortcuts);
#if HAVE_GLOBALACCEL
        case GlobalPrimary:
            return primarySequence(defaultGlobalShortcuts);
        case GlobalAlternate:
            return alternateSequence(defaultGlobalShortcuts);
#endif
        default:
            // Column not valid for this role
            Q_ASSERT(false);
            return QVariant();
        }
    }
    case ObjectRole:
        return QVariant::fromValue(static_cast<QObject *>(m_action));

    default:
        break;
    }

    return QVariant();
}

bool KShortcutsEditorItem::operator<(const QTreeWidgetItem &other) const
{
    const int column = treeWidget() ? treeWidget()->sortColumn() : 0;
    return m_collator.compare(text(column), other.text(column)) < 0;
}

QKeySequence KShortcutsEditorItem::keySequence(uint column) const
{
    QList<QKeySequence> shortcuts = m_action->shortcuts();
#if HAVE_GLOBALACCEL
    QList<QKeySequence> globalShortcuts = KGlobalAccel::self()->shortcut(m_action);
#endif

    switch (column) {
    case LocalPrimary:
        return primarySequence(shortcuts);
    case LocalAlternate:
        return alternateSequence(shortcuts);
#if HAVE_GLOBALACCEL
    case GlobalPrimary:
        return primarySequence(globalShortcuts);
    case GlobalAlternate:
        return alternateSequence(globalShortcuts);
#endif
    default:
        return QKeySequence();
    }
}

void KShortcutsEditorItem::setKeySequence(uint column, const QKeySequence &seq)
{
    QList<QKeySequence> ks;
#if HAVE_GLOBALACCEL
    if (column == GlobalPrimary || column == GlobalAlternate) {
        ks = KGlobalAccel::self()->shortcut(m_action);
        if (!m_oldGlobalShortcut) {
            m_oldGlobalShortcut = new QList<QKeySequence>(ks);
        }
    } else
#endif
    {
        ks = m_action->shortcuts();
        if (!m_oldLocalShortcut) {
            m_oldLocalShortcut = new QList<QKeySequence>(ks);
        }
    }

    if (column == LocalAlternate || column == GlobalAlternate) {
        if (ks.isEmpty()) {
            ks << QKeySequence();
        }

        if (ks.size() <= 1) {
            ks << seq;
        } else {
            ks[1] = seq;
        }
    } else {
        if (ks.isEmpty()) {
            ks << seq;
        } else {
            ks[0] = seq;
        }
    }

    //avoid also setting the default shortcut - what we are setting here is custom by definition
#if HAVE_GLOBALACCEL
    if (column == GlobalPrimary || column == GlobalAlternate) {
        KGlobalAccel::self()->setShortcut(m_action, ks, KGlobalAccel::NoAutoloading);

    } else
#endif
    {
        m_action->setShortcuts(ks);
    }

    updateModified();
}

//our definition of modified is "modified since the chooser was shown".
void KShortcutsEditorItem::updateModified()
{
    if (m_oldLocalShortcut && *m_oldLocalShortcut == m_action->shortcuts()) {
        delete m_oldLocalShortcut;
        m_oldLocalShortcut = nullptr;
    }
#if HAVE_GLOBALACCEL
    if (m_oldGlobalShortcut && *m_oldGlobalShortcut == KGlobalAccel::self()->shortcut(m_action)) {
        delete m_oldGlobalShortcut;
        m_oldGlobalShortcut = nullptr;
    }
#endif
}

bool KShortcutsEditorItem::isModified(uint column) const
{
    switch (column) {
    case Name:
        return false;
    case LocalPrimary:
    case LocalAlternate:
        if (!m_oldLocalShortcut) {
            return false;
        }
        if (column == LocalPrimary) {
            return primarySequence(*m_oldLocalShortcut) != primarySequence(m_action->shortcuts());
        } else {
            return alternateSequence(*m_oldLocalShortcut) != alternateSequence(m_action->shortcuts());
        }
#if HAVE_GLOBALACCEL
    case GlobalPrimary:
    case GlobalAlternate:
        if (!m_oldGlobalShortcut) {
            return false;
        }
        if (column == GlobalPrimary) {
            return primarySequence(*m_oldGlobalShortcut) != primarySequence(KGlobalAccel::self()->shortcut(m_action));
        } else {
            return alternateSequence(*m_oldGlobalShortcut) != alternateSequence(KGlobalAccel::self()->shortcut(m_action));
        }
#endif
    default:
        return false;
    }
}

void KShortcutsEditorItem::undo()
{
    if (m_oldLocalShortcut) {
        // We only ever reset the active Shortcut
        m_action->setShortcuts(*m_oldLocalShortcut);
    }

#if HAVE_GLOBALACCEL
    if (m_oldGlobalShortcut) {
        KGlobalAccel::self()->setShortcut(m_action, *m_oldGlobalShortcut, KGlobalAccel::NoAutoloading);
    }
#endif

    updateModified();
}

void KShortcutsEditorItem::commit()
{
    delete m_oldLocalShortcut;
    m_oldLocalShortcut = nullptr;
    delete m_oldGlobalShortcut;
    m_oldGlobalShortcut = nullptr;
}
