/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOGGLETOOLBARACTION_H
#define KTOGGLETOOLBARACTION_H

#include <KToggleAction>
#include <kxmlgui_export.h>
#include <memory>

class KToolBar;

/*!
 * \class KToggleToolBarAction
 * \inmodule KXmlGui
 *
 * An action that takes care of everything associated with
 * showing or hiding a toolbar by a menu action. It will
 * show or hide the toolbar with the given name when
 * activated, and check or uncheck itself if the toolbar
 * is manually shown or hidden.
 *
 * If you need to perform some additional action when the
 * toolbar is shown or hidden, connect to the toggled(bool)
 * signal. It will be emitted after the toolbar's
 * visibility has changed, whenever it changes.
 */
class KXMLGUI_EXPORT KToggleToolBarAction : public KToggleAction
{
    Q_OBJECT

public:
    /*!
     * \brief Creates a KToggleToolBarAction that manages the \a toolBar
     * with \a text in its label, as a child of \a parent.
     */
    KToggleToolBarAction(KToolBar *toolBar, const QString &text, QObject *parent);

    /*!
     * \brief Destroys the toggle toolbar action.
     */
    ~KToggleToolBarAction() override;

    /*!
     * \brief Returns a pointer to the tool bar it manages.
     */
    KToolBar *toolBar();

    /*!
     * \brief Filters then returns the specified \a event
     * for a given \a watched object.
     *
     * Reimplemented from QObject.
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

private Q_SLOTS:
    void slotToggled(bool checked) override;

private:
    std::unique_ptr<class KToggleToolBarActionPrivate> const d;
};

#endif
