/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#include "ktooltiphelper.h"
#include "ktooltiphelper_p.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QHelpEvent>
#include <QMenu>
#include <QtGlobal>
#include <QToolButton>
#include <QToolTip>
#include <QWhatsThis>
#include <QWhatsThisClickedEvent>

KToolTipHelper *KToolTipHelper::instance()
{
    return KToolTipHelperPrivate::instance();
}

KToolTipHelper *KToolTipHelperPrivate::instance()
{
    if (!s_instance) {
        s_instance = new KToolTipHelper(qApp);
    }
    return s_instance;
}

KToolTipHelper::KToolTipHelper(QObject* parent)
    : QObject{parent},
      d_ptr{new KToolTipHelperPrivate(this)}
{
}

KToolTipHelperPrivate::KToolTipHelperPrivate(KToolTipHelper *q)
    : q_ptr{q}
{
}

KToolTipHelper::~KToolTipHelper() = default;

KToolTipHelperPrivate::~KToolTipHelperPrivate() = default;

bool KToolTipHelper::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KToolTipHelper);
    return d->eventFilter(watched, event);
}

bool KToolTipHelperPrivate::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    switch (event->type()) {
        case QEvent::KeyPress: {
            return handleKeyPressEvent(event);
        }
        case QEvent::ToolTip: {
            return handleToolTipEvent(event);
        }
        case QEvent::WhatsThisClicked: {
            return handleWhatsThisClickedEvent(event);
        }
        default: {
            return false;
        }
    }
}

bool KToolTipHelperPrivate::handleKeyPressEvent(QEvent *event)
{
    if (!QToolTip::isVisible()
        || static_cast<QKeyEvent *>(event)->key() != Qt::Key_Shift
        || !m_widget
    ) {
        return false;
    }
    QToolTip::hideText();
    Q_CHECK_PTR(m_globalPos.get());

    qDebug("toolTip: %s, whatsThis: %s", qPrintable(m_widget->toolTip()), qPrintable(m_widget->whatsThis()));
    if (QMenu *menu = qobject_cast<QMenu *>(m_widget)) {
        if (menu->activeAction() != nullptr) {
            QWhatsThis::showText(*m_globalPos.get(), menu->activeAction()->whatsThis(), m_widget);
        }
    } else {
        QWhatsThis::showText(*m_globalPos.get(), m_widget->whatsThis(), m_widget);
    }
    return true;
}


bool KToolTipHelperPrivate::handleToolTipEvent(QEvent *event)
{
    QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
    m_widget = m_application->widgetAt(helpEvent->globalPos());
    if (!m_widget) {
        return false;
    }
    qDebug("toolTip: %s, whatsThis: %s", qPrintable(m_widget->toolTip()), qPrintable(m_widget->whatsThis()));
    if (m_ignoredWidgets.count(m_widget) > 0) {
        m_widget = nullptr;
        return false;
    }
    if (QToolButton *toolButton = qobject_cast<QToolButton *>(m_widget)) {
        if (auto action = toolButton->defaultAction()) {
            if (!action->shortcut().isEmpty()) {
                toolButton->setToolTip(action->toolTip() + QStringLiteral(" (") + action->shortcut().toString(QKeySequence::NativeText) + QStringLiteral(")"));
            }
        }
    }
    QMenu *menu = nullptr;
    if (m_widget->toolTip().isEmpty() || m_widget->whatsThis().isEmpty()) {
        menu = qobject_cast<QMenu *>(m_widget);
        if (menu) {
            QAction *action = menu->activeAction();
            if (action) {
                // All actions have their text as a tooltip by default.
                // We only want to display the tooltip if it isn't identical
                // to the already visible text in the menu.
                if (hasExplicitToolTip(action)) {
                    if (action->whatsThis().isEmpty()) {
                        QToolTip::showText(helpEvent->globalPos(), action->toolTip());
                    } else {
                        showExpandableToolTip(helpEvent->globalPos(), action->toolTip());
                    }
                    return true;
                } else if (!action->whatsThis().isEmpty()) {
                    showExpandableToolTip(helpEvent->globalPos());
                    return true;
                }
                QToolTip::hideText();
                return false;
            } else {
                QToolTip::hideText();
            }
        }
        return false;
    }
    showExpandableToolTip(helpEvent->globalPos(), m_widget->toolTip());
    return true;
}

bool KToolTipHelperPrivate::handleWhatsThisClickedEvent(QEvent *event)
{
    event->accept();
    const QWhatsThisClickedEvent *whatsThisClickedEvent = static_cast<QWhatsThisClickedEvent *>(event);
    QDesktopServices::openUrl(QUrl(whatsThisClickedEvent->href()));
    return true;
}

void KToolTipHelperPrivate::showExpandableToolTip(const QPoint &globalPos, const QString &toolTip)
{
    m_globalPos.reset(new QPoint(globalPos));
    //qDebug("Sending tooltip");
    KColorScheme colorScheme = KColorScheme(QPalette::Normal, KColorScheme::Tooltip);
    const QColor hintTextColor = colorScheme.foreground(KColorScheme::InactiveText).color();
    if (!toolTip.isEmpty()) {
        QToolTip::showText(*m_globalPos.get(), toolTip + xi18nc("@info:tooltip",
                "<nl/><small><font color=\"%1\">Press <shortcut>Shift</shortcut> "
                "for help.</font></small>", hintTextColor.name()));
    } else {
        QToolTip::showText(*m_globalPos.get(), xi18nc("@info:tooltip",
                "<small><font color=\"%1\">Press <shortcut>Shift</shortcut> "
                "for help.</font></small>", hintTextColor.name()));
    }
}

KToolTipHelper *KToolTipHelperPrivate::s_instance = nullptr;

bool hasExplicitToolTip(const QAction *action)
{
    Q_CHECK_PTR(action);
    const QString iconText = action->iconText();
    const QString toolTip = action->toolTip();
    int i = -1, j = -1;
    do {
        i++; j++;
        // Both of these QStrings are considered equal if their only differences are '&' and '.' chars.
        // Now move both of their indices to the first char that is neither '&' nor '.'.
        while (i < iconText.size()
            && (iconText.at(i) == QLatin1Char('&') || iconText.at(i) == QLatin1Char('.'))) {
            i++;
        }
        while (j < toolTip.size()
            && (toolTip.at(j) == QLatin1Char('&') || toolTip.at(j) == QLatin1Char('.'))) {
            j++;
        }

        if (i >= iconText.size()) {
            return j < toolTip.size();
        }
        if (j >= toolTip.size()) {
            return i < iconText.size();
        }
    } while (iconText.at(i) == toolTip.at(j));
    return true; // We have found a difference.
}
