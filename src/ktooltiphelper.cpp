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

KToolTipHelper::KToolTipHelper(QObject *parent)
    : QObject{parent},
      d{new KToolTipHelperPrivate(this)}
{
}

KToolTipHelperPrivate::KToolTipHelperPrivate(KToolTipHelper *q)
    : q{q}
{
}

KToolTipHelper::~KToolTipHelper() = default;

KToolTipHelperPrivate::~KToolTipHelperPrivate() = default;

bool KToolTipHelper::eventFilter(QObject *watched, QEvent *event)
{
    return d->eventFilter(watched, event);
}

bool KToolTipHelperPrivate::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        return handleKeyPressEvent(event);
    case QEvent::ToolTip:
        return handleToolTipEvent(static_cast<QWidget *>(watched),
                                  static_cast<QHelpEvent *>(event));
    case QEvent::WhatsThisClicked:
        return handleWhatsThisClickedEvent(event);
    default:
        return false;
    }
}

const QString KToolTipHelper::whatsThisHintOnly()
{
    return KToolTipHelperPrivate::whatsThisHintOnly();
}

const QString KToolTipHelperPrivate::whatsThisHintOnly()
{
    return QStringLiteral("tooltip bug"); // if a user ever sees this, there is a bug somewhere.
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

    if (qobject_cast<QMenu *>(m_widget)) {
        if (m_action) {
            QWhatsThis::showText(m_lastExpandableToolTipGlobalPos, m_action->whatsThis(), m_widget);
        }
        return true;
    }
    QWhatsThis::showText(m_lastExpandableToolTipGlobalPos, m_widget->whatsThis(), m_widget);
    return true;
}

bool KToolTipHelperPrivate::handleMenuToolTipEvent(QMenu *menu, QHelpEvent *helpEvent)
{
    Q_CHECK_PTR(helpEvent);
    Q_CHECK_PTR(menu);

    m_action = menu->actionAt(helpEvent->pos());
    if (!m_action || (m_action->menu() && !m_action->menu()->isEmpty())) {
        // Do not show a tooltip when there is a menu since they will compete space-wise.
        QToolTip::hideText();
        return false;
    }

    // All actions have their text as a tooltip by default.
    // We only want to display the tooltip text if it isn't identical
    // to the already visible text in the menu.
    const bool explicitToolTip = hasExplicitToolTip(m_action);
    // We only want to show the whatsThisHint in a tooltip if the whatsThis isn't empty.
    const bool emptyWhatsThis = m_action->whatsThis().isEmpty();
    if (!explicitToolTip && emptyWhatsThis) {
        QToolTip::hideText();
        return false;
    }

    // Calculate a nice location for the tooltip so it doesn't unnecessarily cover
    // a part of the menu.
    const QRect actionGeometry = menu->actionGeometry(m_action);
    const int xOffset = menu->layoutDirection() == Qt::RightToLeft ? 0 : actionGeometry.width();
    const QPoint toolTipPosition(
            helpEvent->globalX() - helpEvent->x() + xOffset,
            helpEvent->globalY() - helpEvent->y() + actionGeometry.y() - actionGeometry.height() / 2);

    if (explicitToolTip) {
        if (emptyWhatsThis) {
            if (m_action->toolTip() != whatsThisHintOnly()) {
                QToolTip::showText(toolTipPosition, m_action->toolTip(), m_widget, actionGeometry);
            }
        } else {
            showExpandableToolTip(toolTipPosition, m_action->toolTip(), actionGeometry);
        }
        return true;
    }
    Q_ASSERT(!m_action->whatsThis().isEmpty());
    showExpandableToolTip(toolTipPosition, QStringLiteral(), actionGeometry);
    return true;
}

bool KToolTipHelperPrivate::handleToolTipEvent(QWidget *watchedWidget, QHelpEvent *helpEvent)
{
    m_widget = watchedWidget;

    if (QToolButton *toolButton = qobject_cast<QToolButton *>(m_widget)) {
        if (const QAction *action = toolButton->defaultAction()) {
            if (!action->shortcut().isEmpty() && action->toolTip() != whatsThisHintOnly()) {
                toolButton->setToolTip(i18nc(
                        "@info:tooltip %1 is the tooltip of an action, %2 is its keyboard shorcut",
                        "%1 (%2)",
                        action->toolTip(), action->shortcut().toString(QKeySequence::NativeText)));
                // Do not replace the brackets in the above i18n-call with <shortcut> tags from
                // KUIT because mixing KUIT with HTML is not allowed and %1 could be anything.

                // We don't show the tooltip here because aside from adding the keyboard shortcut
                // the QToolButton can now be handled like the tooltip event for any other widget.
            }
        }
    } else if (QMenu *menu = qobject_cast<QMenu *>(m_widget)) {
        return handleMenuToolTipEvent(menu, helpEvent);
    }

    while (m_widget->toolTip().isEmpty()) {
        m_widget = m_widget->parentWidget();
        if (!m_widget) {
            return false;
        }
    }
    if (m_widget->whatsThis().isEmpty()) {
        if (m_widget->toolTip() == whatsThisHintOnly()) {
            return true;
        }
        return false;
    }
    showExpandableToolTip(helpEvent->globalPos(), m_widget->toolTip());
    return true;
}

bool KToolTipHelperPrivate::handleWhatsThisClickedEvent(QEvent *event)
{
    event->accept();
    const auto whatsThisClickedEvent = static_cast<QWhatsThisClickedEvent *>(event);
    QDesktopServices::openUrl(QUrl(whatsThisClickedEvent->href()));
    return true;
}

void KToolTipHelperPrivate::showExpandableToolTip(const QPoint &globalPos,
                                                  const QString &toolTip,
                                                  const QRect &rect)
{
    m_lastExpandableToolTipGlobalPos = QPoint(globalPos);
    const KColorScheme colorScheme = KColorScheme(QPalette::Normal, KColorScheme::Tooltip);
    const QColor hintTextColor = colorScheme.foreground(KColorScheme::InactiveText).color();

    if (toolTip.isEmpty() || toolTip == whatsThisHintOnly()) {
        // i18n: @info:tooltip Pressing Shift will show a longer message with contextual info
        // about the thing the tooltip was invoked for. If there is no good way to translate
        // the message, translating "Press Shift to learn more." would also mostly fit what
        // is supposed to be expressed here.
        const QString whatsThisHint = i18n(
                "<small><font color=\"%1\">Press <b>Shift</b> for more Info.</font></small>",
                hintTextColor.name());
        QToolTip::showText(m_lastExpandableToolTipGlobalPos, whatsThisHint, m_widget, rect);
    } else {
        // i18n: @info:tooltip The 'Press Shift for more' message is added to tooltips that have an
        // available whatsthis help message. Pressing Shift will show this more exhaustive message.
        // It is particularly important to keep this translation short because:
        // 1. A longer translation will increase the size of *every* tooltip that gets this hint
        //      added e.g. a two word tooltip followed by a four word hint.
        // 2. The purpose of this hint is so we can keep the tooltip shorter than it would have to
        //      be if we couldn't refer to the message that appears when pressing Shift.
        //
        // %1 can be any tooltip. <br/> produces a linebreak. The other things between < and > are
        // styling information. The word "more" refers to "information".
        const QString toolTipWithHint = QStringLiteral("<qt>") + i18n(
                "%1<br/><small><font color=\"%2\">Press <b>Shift</b> for more.</font></small>",
                toolTip, hintTextColor.name()) + QStringLiteral("</qt>");
        // Do not replace above HTML tags with KUIT because mixing HTML and KUIT is not allowed and
        // we can not know what kind of markup the tooltip in %1 contains.
        QToolTip::showText(m_lastExpandableToolTipGlobalPos, toolTipWithHint, m_widget, rect);
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
        // Now move both of their indices to the next char that is neither '&' nor '.'.
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
