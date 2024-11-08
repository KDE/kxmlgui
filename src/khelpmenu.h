/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 1999-2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KHELPMENU_H
#define KHELPMENU_H

#include <kxmlgui_export.h>

#include <QObject>
#include <QString>

class QMenu;
class QWidget;
class QAction;

class KAboutData;
class KHelpMenuPrivate;

/**
 * @class KHelpMenu khelpmenu.h KHelpMenu
 *
 * @short Standard %KDE help menu with dialog boxes.
 *
 * This class provides the standard %KDE help menu with the default "about"
 * dialog boxes and help entry.
 *
 * This class is used in KMainWindow so
 * normally you don't need to use this class yourself. However, if you
 * need the help menu or any of its dialog boxes in your code that is
 * not subclassed from KMainWindow you should use this class.
 *
 * The usage is simple:
 *
 * \code
 * mHelpMenu = new KHelpMenu( this );
 * kmenubar->addMenu(mHelpMenu->menu() );
 * \endcode
 *
 * or if you just want to open a dialog box:
 *
 * \code
 * mHelpMenu = new KHelpMenu( this );
 * connect(this, &ClassFoo::someSignal, mHelpMenu, &KHelpMenu::aboutKDE);
 * \endcode
 *
 * IMPORTANT:
 * The first time you use KHelpMenu::menu(), a QMenu object is
 * allocated. Only one object is created by the class so if you call
 * KHelpMenu::menu() twice or more, the same pointer is returned. The class
 * will destroy the popupmenu in the destructor so do not delete this
 * pointer yourself.
 *
 * The KHelpMenu object will be deleted when its parent is destroyed but you
 * can delete it yourself if you want. The code below will always work.
 *
 * \code
 * MyClass::~MyClass()
 * {
 *   delete mHelpMenu;
 * }
 * \endcode
 *
 *
 * Using your own "about application" dialog box:
 *
 * The standard "about application" dialog box is quite simple. If you
 * need a dialog box with more functionality you must design that one
 * yourself. When you want to display the dialog, you simply need to
 * connect the help menu signal showAboutApplication() to your slot.
 *
 * \code
 * void MyClass::myFunc()
 * {
 *   ..
 *   KHelpMenu *helpMenu = new KHelpMenu( this );
 *   connect(helpMenu, &KHelpMenu::showAboutApplication, this, &ClassFoo:myDialogSlot);
 *   ..
 * }
 *
 * void MyClass::myDialogSlot()
 * {
 *   <activate your custom dialog>
 * }
 * \endcode
 *
 * \image html khelpmenu.png "KHelpMenu"
 *
 * KHelpMenu respects Kiosk settings (see the KAuthorized namespace in the
 * KConfig framework).  In particular, system administrators can disable items
 * on this menu using some subset of the following configuration:
 * @verbatim
   [KDE Action Restrictions][$i]
   actions/help_contents=false
   actions/help_whats_this=false
   actions/help_report_bug=false
   actions/switch_application_language=false
   actions/help_about_app=false
   actions/help_about_kde=false
   @endverbatim
 *
 * @author Espen Sand (espen@kde.org)
 */

class KXMLGUI_EXPORT KHelpMenu : public QObject
{
    Q_OBJECT

public:
#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    /**
     * Constructor.
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     * @param unused This value is unused.
     * @param showWhatsThis Decides whether a "What's this" entry will be
     *        added to the dialog.
     *
     * @deprecated Since 6.9, use one of the other constructors
     */
    KXMLGUI_DEPRECATED_VERSION(6, 9, "Use one of the other constructors")
    explicit KHelpMenu(QWidget *parent, const QString &unused, bool showWhatsThis = true);
#endif

    /**
     * Creates a KHelpMenu with the default app data (KAboutData::applicationData()).
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     *
     * @since 6.9
     */
    explicit KHelpMenu(QWidget *parent = nullptr);

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    /**
     * Constructor.
     *
     * This alternative constructor is mainly useful if you want to
     * override the standard actions (aboutApplication(), aboutKDE(),
     * helpContents(), reportBug, and optionally whatsThis).
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     * @param aboutData User and app data used in the About app dialog
     * @param showWhatsThis Decides whether a "What's this" entry will be
     *        added to the dialog.
     *
     * @deprecated Since 6.9, use one of the other constructors
     */
    KXMLGUI_DEPRECATED_VERSION(6, 9, "Use one of the other constructors")
    KHelpMenu(QWidget *parent, const KAboutData &aboutData, bool showWhatsThis);
#endif

    /**
     * Creates a KHelpMenu with app data @p aboutData.
     *
     * This constructor is useful if you want to use the help menu with custom
     * application data.
     *
     * @param parent The parent of the dialog boxes. The boxes are modeless
     *        and will be centered with respect to the parent.
     * @param aboutData User and app data used in the About app dialog and for
     *        the actions in the help menu.
     *
     * @since 6.9
     */
    KHelpMenu(QWidget *parent, const KAboutData &aboutData);

    /**
     * Destructor
     *
     * Destroys dialogs and the menu pointer returned by menu
     */
    ~KHelpMenu() override;

    /**
     * Set whether to show the What's This menu entry in the help menu.
     * The default is to show the menu entry (if Kiosk settings allow it).
     * @since 6.9
     */
    void setShowWhatsThis(bool showWhatsThis);

    /**
     * Returns a popup menu you can use in the menu bar or where you
     * need it.
     *
     * The returned menu is configured with an icon, a title and
     * menu entries. Therefore adding the returned pointer to your menu
     * is enough to have access to the help menu.
     *
     * Note: This method will only create one instance of the menu. If
     * you call this method twice or more the same pointer is returned.
     */
    QMenu *menu();

    enum MenuId {
        menuHelpContents = 0,
        menuWhatsThis = 1,
        menuAboutApp = 2,
        menuAboutKDE = 3,
        menuReportBug = 4,
        menuSwitchLanguage = 5,
        menuDonate = 6, ///< @since 5.24
    };

    /**
     * Returns the QAction * associated with the given parameter
     * Will return a nullptr if menu() has not been called
     *
     * @param id The id of the action of which you want to get QAction *
     */
    QAction *action(MenuId id) const;

public Q_SLOTS:
    /**
     * Opens the help page for the application. The application name is
     * used as a key to determine what to display and the system will attempt
     * to open \<appName\>/index.html.
     */
    void appHelpActivated();

    /**
     * Activates What's This help for the application.
     */
    void contextHelpActivated();

    /**
     * Opens an application specific dialog box.
     *
     * The method will try to open the about box using the following steps:
     * - If the showAboutApplication() signal is connected, then it will be emitted.
     *   This means there is an application defined aboutBox.
     * - If the aboutData was set in the constructor a KAboutApplicationDialog will be created
     *   with this aboutData.
     * - Else a KAboutApplicationDialog will be created using KAboutData::applicationData().
     */
    void aboutApplication();

    /**
     * Opens the standard "About KDE" dialog box.
     */
    void aboutKDE();

    /**
     * Opens the standard "Report Bugs" dialog box.
     */
    void reportBug();

    /**
     * Opens the changing default application language dialog box.
     */
    void switchApplicationLanguage();

    /**
     * Opens the donate url.
     * @since 5.24
     */
    void donate();

private Q_SLOTS:
    /**
     * Connected to the menu pointer (if created) to detect a delete
     * operation on the pointer. You should not delete the pointer in your
     * code yourself. Let the KHelpMenu destructor do the job.
     */
    KXMLGUI_NO_EXPORT void menuDestroyed();

    /**
     * Connected to the dialogs (about kde and bug report) to detect
     * when they are finished.
     */
    KXMLGUI_NO_EXPORT void dialogFinished();

    /**
     * This slot will delete a dialog (about kde or bug report) if the
     * dialog pointer is not zero and the dialog is not visible. This
     * slot is activated by a one shot timer started in dialogHidden
     */
    KXMLGUI_NO_EXPORT void timerExpired();

Q_SIGNALS:
    /**
     * This signal is emitted from aboutApplication() if no
     * "about application" string has been defined. The standard
     * application specific dialog box that is normally activated in
     * aboutApplication() will not be displayed when this signal
     * is emitted.
     */
    void showAboutApplication();

private:
    KHelpMenuPrivate *const d;
};

#endif
