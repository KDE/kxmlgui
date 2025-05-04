/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KEDITTOOLBAR_H
#define KEDITTOOLBAR_H

#include <QDialog>
#include <memory>

#include <kxmlgui_export.h>

class KActionCollection;

class KEditToolBarPrivate;
class KXMLGUIFactory;
/*!
 * \class KEditToolBar
 * \inmodule KXmlGui
 *
 * \brief A dialog used to customize or configure toolbars.
 *
 * This dialog only works if your application uses the XML UI
 * framework for creating menus and toolbars.  It depends on the XML
 * files to describe the toolbar layouts and it requires the actions
 * to determine which buttons are active.
 *
 * Typically you do not need to use it directly as KXmlGuiWindow::setupGUI
 * takes care of it.
 *
 * If you use KXMLGUIClient::plugActionList() you need to overload
 * KXmlGuiWindow::saveNewToolbarConfig() to plug actions again:
 *
 * \code
 * void MyClass::saveNewToolbarConfig()
 * {
 *     KXmlGuiWindow::saveNewToolbarConfig();
 *     plugActionList( "list1", list1Actions );
 *     plugActionList( "list2", list2Actions );
 * }
 * \endcode
 *
 * When created, KEditToolBar takes a KXMLGUIFactory object, and uses it to
 * find all of the action collections and XML files (there is one of each for the
 * mainwindow, but there could be more, when adding other XMLGUI clients like
 * KParts or plugins). The editor aims to be semi-intelligent about where it
 * assigns any modifications. In other words, it will not write out part specific
 * changes to your application's main XML file.
 *
 * KXmlGuiWindow and KParts::MainWindow take care of creating KEditToolBar correctly
 * and connecting to its newToolBarConfig slot, but if you really really want to do it
 * yourself, see the KXmlGuiWindow::configureToolbars() and
 * KXmlGuiWindow::saveNewToolbarConfig() code.
 *
 * \image kedittoolbar.png "KEditToolBar (example: usage in KWrite)"
 *
 */
class KXMLGUI_EXPORT KEditToolBar : public QDialog
{
    Q_OBJECT
public:
    /*!
     * \brief Old constructor for apps that do not use components.
     *
     * This constructor is somewhat deprecated, since it doesn't work
     * with any KXMLGuiClient being added to the mainwindow.
     * You really want to use the other constructor.
     *
     * You \b must pass along your collection of actions (some of which appear in your toolbars).
     *
     * \a collection The collection of actions to work on.
     *
     * \a parent The parent of the dialog.
     */
    explicit KEditToolBar(KActionCollection *collection, QWidget *parent = nullptr);

    /*!
     * \brief Main constructor.
     *
     * The main parameter, \a factory, is a pointer to the
     * XML GUI factory object for your application.  It contains a list
     * of all of the GUI clients (along with the action collections and
     * xml files) and the toolbar editor uses that.
     *
     * Use this like so:
     * \code
     * KEditToolBar edit(factory());
     * if (edit.exec())
     * // ...
     * \endcode
     *
     * \a factory Your application's factory object.
     *
     * \a parent The usual parent for the dialog.
     */
    explicit KEditToolBar(KXMLGUIFactory *factory, QWidget *parent = nullptr);

    /*!
     * \brief Destructor.
     */
    ~KEditToolBar() override;

    /*!
     * \brief Sets \a toolBarName as the default toolbar
     * that will be selected when the dialog is shown.
     *
     * If not set, or QString() is passed in, the global default tool bar name
     * will be used.
     *
     * \sa setGlobalDefaultToolBar
     */
    void setDefaultToolBar(const QString &toolBarName);

    /*!
     * \brief Sets a new resource \a file and whether the global resource file
     * should be used.
     *
     * The name (absolute or relative) of your application's UI resource \a file
     * is assumed to be share/apps/appname/appnameui.rc, but it can be
     * overridden by calling this method.
     *
     * The \a global parameter controls whether or not the
     * global resource file is used.  If this is \c true, then you may
     * edit all of the actions in your toolbars -- global ones and
     * local one.  If it is \c false, then you may edit only your
     * application's entries.  The only time you should set this to
     * false is if your application does not use the global resource
     * file at all (very rare).
     *
     * \a file The application's local resource file.
     *
     * \a global If \c true, then the global resource file will also be parsed.
     */
    void setResourceFile(const QString &file, bool global = true);

    /*!
     * \brief Sets \a toolBarName as the default toolbar
     * which will be auto-selected for all KEditToolBar instances.
     *
     * Can be overridden on a per-dialog basis
     * by calling setDefaultToolBar( const QString& ) on the dialog.
     *
     * \since 6.0
     */
    static void setGlobalDefaultToolBar(const QString &toolBarName);

Q_SIGNALS:
    /*!
     * \brief Emitted when 'Apply' or 'Ok' is clicked or toolbars were reset.
     *
     * Connect to it to plug action lists and to call applyMainWindowSettings
     * (see sample code in the documentation for this class).
     */
    void newToolBarConfig();

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    friend class KEditToolBarPrivate;
    std::unique_ptr<KEditToolBarPrivate> const d;

    Q_DISABLE_COPY(KEditToolBar)
};

#endif // _KEDITTOOLBAR_H
