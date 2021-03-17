/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#ifndef KTOOLTIPHELPER_P_H
#define KTOOLTIPHELPER_P_H

#include <qobject.h>

#include <QPointer>
#include <QRect>

class KToolTipHelper;

class QAction;
class QApplication;
class QHelpEvent;
class QMenu;

/**
 * The private class of KToolTipHelper used for the PIMPL idiom.
 * \internal
 */
class KToolTipHelperPrivate : public QObject
{
    Q_OBJECT

public:
    /**
     * Singleton implementation for KToolTipHelper and
     * NOT of this class (KToolTipHelperPrivate).
     */
    static KToolTipHelper *instance();

    explicit KToolTipHelperPrivate(KToolTipHelper *q);

    virtual ~KToolTipHelperPrivate();

    /** @see KToolTipHelper::eventFilter() */
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    /** @see KToolTipHelper::whatsThisHintOnly() */
    static const QString whatsThisHintOnly();

    /**
     * @return true if the key press is used to expand a tooltip. false otherwise.
     */
    bool handleKeyPressEvent(QEvent *event);
    /**
     * Is called from handleToolTipEvent() to handle a QEvent::ToolTip in a menu.
     * This method will show the tooltip of the action that is hovered at a nice
     * position.
     * @param menu      The menu that a tooltip is requested for
     * @param helpEvent The QEvent::ToolTip that was cast to a QHelpEvent
     */
    bool handleMenuToolTipEvent(QMenu *menu, QHelpEvent *helpEvent);
    /**
     * @param watchedWidget The widget that is receiving the QHelpEvent
     * @param helpEvent     The QEvent::ToolTip that was cast to a QHelpEvent
     * @return false if no special handling of the tooltip event seems necessary. true otherwise.
     */
    bool handleToolTipEvent(QWidget *watchedWidget, QHelpEvent *helpEvent);
    /**
     * Handles links being clicked in whatsThis.
     * @return true.
     */
    bool handleWhatsThisClickedEvent(QEvent *event);

    /**
     * Shows a tooltip that contains a whatsThisHint at the location \p globalPos.
     * If \p tooltip is empty, only a whatsThisHint is shown.
     *
     * The parameter usage is identical to that of QToolTip::showText. The only difference
     * is that this method doesn't need a QWidget *w parameter because that one is already
     * retrieved in handleToolTipEvent() prior to calling this method.
     *
     * @see QToolTip::showText()
     */
    void showExpandableToolTip(const QPoint &globalPos, const QString &toolTip = QStringLiteral(), const QRect &rect = QRect());

public:
    KToolTipHelper *const q;

private:
    /** An action in a menu a tooltip was requested for. */
    QPointer<QAction> m_action;
    /** The global position where the last tooltip which had a whatsThisHint was displayed. */
    QPoint m_lastExpandableToolTipGlobalPos;
    /** The last widget a QEvent::tooltip was sent for. */
    QPointer<QWidget> m_widget;

    static KToolTipHelper *s_instance;
};

/**
 * All QActions have their iconText() as their toolTip() by default.
 * This method checks if setToolTip() was called for the action explicitly to set a different/more
 * useful tooltip.
 *
 * @return true if the toolTip() isn't just an automatically generated version of iconText().
 *         false otherwise.
 */
bool hasExplicitToolTip(const QAction *action);

#endif // KTOOLTIPHELPER_P_H
