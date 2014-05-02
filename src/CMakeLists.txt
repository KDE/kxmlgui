add_subdirectory(ksendbugmail)

if (HAVE_ATTICA)
   set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} KF5::Attica)
endif ()
if(WIN32)
    set (XMLGUI_EXTRA_LIBS ${XMLGUI_EXTRA_LIBS} Secur32) # GetUserNameEx()
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
  kactioncategory.cpp
  kactioncollection.cpp
  kactionconflictdetector.cpp
  kbugreport.cpp
  kedittoolbar.cpp
  kgesture.cpp
  kgesturemap.cpp
  khelpmenu.cpp
  kkeysequencewidget.cpp
  kmainwindow.cpp
  kmainwindowiface.cpp
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
  kxmlguibuilder.cpp
  kxmlguiclient.cpp
  kxmlguifactory.cpp
  kxmlguifactory_p.cpp
  kxmlguiversionhandler.cpp
  kxmlguiwindow.cpp
  kundoactions.cpp
  kcheckaccelerators.cpp
)

set(kxmlgui_UI
   kshortcutsdialog.ui
   kshortcutwidget.ui
)

qt5_wrap_ui(kxmlgui_SRCS ${kxmlgui_UI} )

add_library(KF5XmlGui ${kxmlgui_SRCS})
generate_export_header(KF5XmlGui BASE_NAME KXmlGui)
add_library(KF5::XmlGui ALIAS KF5XmlGui)

target_include_directories(KF5XmlGui INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KXmlGui>")

target_link_libraries(KF5XmlGui
PUBLIC
    Qt5::DBus           #QDBus connect to signal in KToolBar
    Qt5::Xml            #To parse the configuration (QDomDocument etc)
    Qt5::Widgets        #QWidget is used everywhere
    KF5::ConfigCore         #Reading config for ToolbarIcons, Shortcut Schemes...
    KF5::ConfigWidgets      #KStandardAction, KToggleAction...
PRIVATE
    Qt5::Network        #QNetworkAccessManager in kaboutapplicationpersonmodel_p
    Qt5::PrintSupport   #QPrinter in kshortcutseditor
    KF5::WidgetsAddons
    KF5::ItemViews           #KWidgetItemDelegate in KAboutApplicationPersonListDelegate
    KF5::I18n                #i18n and i18nc in many places
    KF5::IconThemes         #KIconLoader and KIconThemes in KToolBar
    KF5::TextWidgets        #KTextEdit in kbugreport
    KF5::WindowSystem       #KKeyServer in kkeysequencewidget
    KF5::GlobalAccel         #KActionCollection and shortcut widgets
    ${XMLGUI_EXTRA_LIBS}
)

set_target_properties(KF5XmlGui PROPERTIES VERSION   ${KXMLGUI_VERSION_STRING}
                                           SOVERSION ${KXMLGUI_SOVERSION}
                                           EXPORT_NAME XmlGui
)

ecm_generate_headers(KXmlGui_HEADERS
  HEADER_NAMES
  KAboutApplicationDialog
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
  kgesture_p.h
  kgesturemap_p.h
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_export.h
  ${KXmlGui_HEADERS}
  DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KXmlGui COMPONENT Devel
)

install( FILES ui_standards.rc  DESTINATION  ${CONFIG_INSTALL_DIR}/ui )

install(FILES
 aboutkde.png
 thumb_frame.png
 DESTINATION  ${KF5_DATA_INSTALL_DIR}/kxmlgui/pics )

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KXmlGui LIB_NAME KF5XmlGui DEPS "dbus xml widgets KConfigCore KConfigWidgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KXmlGui)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
