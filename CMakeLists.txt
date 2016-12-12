cmake_minimum_required(VERSION 2.8.12)

project(KXmlGui)

# ECM setup
include(FeatureSummary)
find_package(ECM 5.29.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(ECMMarkNonGuiExecutable)
include(ECMSetupVersion)
include(ECMGenerateHeaders)

include(GenerateExportHeader)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMQtDeclareLoggingCategory)

set(KF5_VERSION "5.30.0") # handled by release scripts
set(KF5_DEP_VERSION "5.29.0") # handled by release scripts

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX KXMLGUI
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlGuiConfigVersion.cmake"
                  SOVERSION 5)

option(FORCE_DISABLE_KGLOBALACCEL "Force building KXmlGui without KGlobalAccel. Doing this will break global shortcut support. [default=OFF]" OFF)

# Dependencies
set(REQUIRED_QT_VERSION 5.5.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets DBus Xml Network PrintSupport)

find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5TextWidgets ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Attica ${KF5_DEP_VERSION})
set_package_properties(KF5Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in KXMLGUI"
                       URL "https://projects.kde.org/attica"
                       TYPE OPTIONAL
                      )
set (HAVE_ATTICA ${KF5Attica_FOUND})

if (NOT FORCE_DISABLE_KGLOBALACCEL)
    find_package(KF5GlobalAccel ${KF5_DEP_VERSION} REQUIRED)
endif()
set (HAVE_GLOBALACCEL ${KF5GlobalAccel_FOUND})

# Subdirectories
add_definitions(-DTRANSLATION_DOMAIN=\"kxmlgui5\")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(autotests)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5XmlGui")

include(ECMPackageConfigHelpers)

ecm_configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5XmlGuiConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlGuiConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlGuiConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlGuiConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5XmlGuiTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5XmlGuiTargets.cmake NAMESPACE KF5:: )

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
)

feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)
