add_subdirectory(ksendbugmail)

if (HAVE_ATTICA)
   set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} KF5::Attica)
endif ()
if (HAVE_GLOBALACCEL)
   set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} KF5::GlobalAccel)
endif ()
if(WIN32)
    set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} secur32) # GetUserNameEx()
endif()

configure_file(config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kaboutapplicationconfigattica_p.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kaboutapplicationconfigattica_p.h
)

set(kxmlgui_SRCS
  kaboutapplicationdialog.cpp
  kaboutapplicationpersonlistdelegate_p.cpp
  kaboutapplicationpersonlistview_p.cpp
  kaboutapplicationpersonmodel_p.cpp
  kaboutkdedialog_p.cpp
  kaboutplugindialog.cpp
  kabstractaboutdialog_p.cpp
  kactioncategory.cpp
  kactioncollection.cpp
  kactionconflictdetector.cpp
  kbugreport.cpp
  kedittoolbar.cpp
  khelpmenu.cpp
  kkeysequencewidget.cpp
  klicensedialog_p.cpp
  kmainwindow.cpp
  kmenumenuhandler_p.cpp
  kshortcuteditwidget.cpp
  kshortcutschemeseditor.cpp
  kshortcutschemeshelper.cpp
  kshortcutsdialog.cpp
  kshortcutseditor.cpp
  kshortcutseditordelegate.cpp
  kshortcutseditoritem.cpp
  kshortcutwidget.cpp
  kswitchlanguagedialog_p.cpp
  ktoggletoolbaraction.cpp
  ktoolbar.cpp
  ktoolbarhandler.cpp
  ktoolbarhelper.cpp
  kxmlguibuilder.cpp
  kxmlguiclient.cpp
  kxmlguifactory.cpp
  kxmlguifactory_p.cpp
  kxmlguiversionhandler.cpp
  kxmlguiwindow.cpp
  kundoactions.cpp
  kcheckaccelerators.cpp
  waylandshortcutinhibitor.cpp
)
if (TARGET Qt5::DBus)
  list(APPEND kxmlgui_SRCS kmainwindowiface.cpp)
endif()

# add the resource file
qt5_add_resources(kxmlgui_SRCS kxmlgui.qrc)

ecm_qt_declare_logging_category(kxmlgui_SRCS
    HEADER debug.h
    IDENTIFIER DEBUG_KXMLGUI
    CATEGORY_NAME kf.xmlgui
    OLD_CATEGORY_NAMES kf5.kxmlgui
    DESCRIPTION "KXmlGui"
    EXPORT KXMLGUI
)

set(kxmlgui_UI
   kshortcutsdialog.ui
   kshortcutwidget.ui
)

ki18n_wrap_ui(kxmlgui_SRCS ${kxmlgui_UI} )

ecm_add_qtwayland_client_protocol(kxmlgui_SRCS
    PROTOCOL keyboard-shortcuts-inhibit-unstable-v1.xml
    BASENAME keyboard-shortcuts-inhibit-unstable-v1
)

add_library(KF5XmlGui ${kxmlgui_SRCS})
ecm_generate_export_header(KF5XmlGui
    BASE_NAME KXmlGui
    GROUP_BASE_NAME KF
    VERSION ${KF5_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 4.1 5.0 5.75
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

add_library(KF5::XmlGui ALIAS KF5XmlGui)

target_include_directories(KF5XmlGui INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlGui>")

target_include_directories(KF5XmlGui PRIVATE ${Qt5Gui_PRIVATE_INCLUDE_DIRS}) # for native interface to get wl_seat and surface

target_link_libraries(KF5XmlGui
PUBLIC
    Qt5::Xml            #To parse the configuration (QDomDocument etc)
    Qt5::Widgets        #QWidget is used everywhere
    KF5::ConfigCore         #Reading config for ToolbarIcons, Shortcut Schemes...
    KF5::ConfigWidgets      #KStandardAction, KToggleAction...
PRIVATE
    Qt5::Network        #QNetworkAccessManager in kaboutapplicationpersonmodel_p
    Qt5::PrintSupport   #QPrinter in kshortcutseditor
    Qt5::CorePrivate    #QSystemLocale in initializeLanguages
    Qt5::WaylandClient
    KF5::CoreAddons     #KAboutData
    KF5::WidgetsAddons
    KF5::ItemViews           #KWidgetItemDelegate in KAboutApplicationPersonListDelegate
    KF5::I18n                #i18n and i18nc in many places
    KF5::IconThemes         #KIconLoader and KIconThemes in KToolBar
    KF5::WindowSystem       #KKeyServer in kkeysequencewidget
    Wayland::Client
    ${XMLGUI_EXTRA_LIBS}
)
if (TARGET Qt5::DBus)
  target_link_libraries(KF5XmlGui PUBLIC Qt5::DBus) #QDBus connect to signal in KToolBar
endif()

set_target_properties(KF5XmlGui PROPERTIES VERSION   ${KXMLGUI_VERSION_STRING}
                                           SOVERSION ${KXMLGUI_SOVERSION}
                                           EXPORT_NAME XmlGui
)

ecm_generate_headers(KXmlGui_HEADERS
  HEADER_NAMES
  KAboutApplicationDialog
  KAboutPluginDialog
  KActionCategory
  KActionCollection
  KBugReport
  KEditToolBar
  KHelpMenu
  KKeySequenceWidget
  KMainWindow
  KShortcutsDialog
  KShortcutsEditor
  KShortcutWidget
  KToggleToolBarAction
  KToolBar
  KXMLGUIBuilder
  KXMLGUIClient
  KXMLGUIFactory
  KXmlGuiWindow
  KUndoActions

  REQUIRED_HEADERS KXmlGui_HEADERS
)

install(TARGETS KF5XmlGui EXPORT KF5XmlGuiTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_export.h
  ${KXmlGui_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlGui COMPONENT Devel
)

# install this file to be compatible, it is bundled in the resource file, too
install( FILES ui_standards.rc  DESTINATION  ${KDE_INSTALL_CONFDIR}/ui )

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

ecm_qt_install_logging_categories(
    EXPORT KXMLGUI
    FILE kxmlgui.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_QCH)
    ecm_add_qch(
        KF5XmlGui_QCH
        NAME KXmlGui
        BASE_NAME KF5XmlGui
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KXmlGui_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Xml_QCH
            Qt5DBus_QCH
            Qt5Widgets_QCH
            KF5Config_QCH
            KF5ConfigWidgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KXMLGUI_EXPORT
            KXMLGUI_DEPRECATED_EXPORT
            KXMLGUI_DEPRECATED
            "KXMLGUI_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KXmlGui LIB_NAME KF5XmlGui DEPS "dbus xml widgets KConfigCore KConfigWidgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KXmlGui)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
