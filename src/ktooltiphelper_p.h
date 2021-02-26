/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#ifndef KTOOLTIPHELPER_P_H
#define KTOOLTIPHELPER_P_H

#include <qobject.h>

#include <QPointer>

#include <memory>
#include <unordered_set>

class KToolTipHelper;

class QAction;
class QApplication;

/**
 * The private class of KToolTipHelper used for the PIMPL idiom.
 */
class KToolTipHelperPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KToolTipHelper)

public:
    /**
     * Singleton implementation of KToolTipHelper but
     * NOT of this class (KToolTipHelperPrivate).
     */
    static KToolTipHelper *instance();

    /**
     * TODO
     */
    explicit KToolTipHelperPrivate(KToolTipHelper *q);

    virtual ~KToolTipHelperPrivate();

    /** @see KToolTipHelper::eventFilter() */
    virtual bool eventFilter(QObject* watched, QEvent* event) override;

    /** @see KToolTipHelper::whatsThisHintOnly() */
    const QString whatsThisHintOnly() const;

    bool handleToolTipEvent(QEvent *event);
    bool handleKeyPressEvent(QEvent *event);
    bool handleWhatsThisClickedEvent(QEvent *event);

    void showExpandableToolTip(const QPoint &globalPos, const QString &toolTip = QStringLiteral());

public:
    KToolTipHelper *const q_ptr;

private:
    QApplication *m_application;
    std::unique_ptr<QPoint> m_globalPos;
    std::unordered_set<QWidget *> m_ignoredWidgets;
    QPointer<QWidget> m_widget;

    static KToolTipHelper *s_instance;
};

/**
 * All actions have their iconText() as their toolTip() by default.
 * This method checks if setToolTip() was called for the action explicitly to set a different/more
 * useful tooltip.
 * @return true if the toolTip() isn't just an automatically generated version of iconText().
 *         false otherwise.
 */
bool hasExplicitToolTip(const QAction *action);

#endif // KTOOLTIPHELPER_P_H
