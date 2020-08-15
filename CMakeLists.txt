cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.74.0") # handled by release scripts
set(KF5_DEP_VERSION "5.73.0") # handled by release scripts
project(KXmlGui VERSION ${KF5_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 5.73.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMMarkNonGuiExecutable)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMGenerateExportHeader)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMQtDeclareLoggingCategory)

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KXMLGUI
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5XmlGuiConfigVersion.cmake"
                  SOVERSION 5)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(FORCE_DISABLE_KGLOBALACCEL "Force building KXmlGui without KGlobalAccel. Doing this will break global shortcut support. [default=OFF]" OFF)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

# Dependencies
set(REQUIRED_QT_VERSION 5.12.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets Xml Network PrintSupport)
if (NOT ANDROID)
  find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

find_package(KF5CoreAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5ItemViews ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Config ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5ConfigWidgets ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5I18n ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5IconThemes ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5WidgetsAddons ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5WindowSystem ${KF5_DEP_VERSION} REQUIRED)
find_package(KF5Attica ${KF5_DEP_VERSION})
set_package_properties(KF5Attica PROPERTIES DESCRIPTION "A Qt library that implements the Open Collaboration Services API"
                       PURPOSE "Support for Get Hot New Stuff in KXMLGUI"
                       URL "https://commits.kde.org/attica"
                       TYPE OPTIONAL
                      )
set (HAVE_ATTICA ${KF5Attica_FOUND})

if (NOT FORCE_DISABLE_KGLOBALACCEL)
    find_package(KF5GlobalAccel ${KF5_DEP_VERSION} REQUIRED)
endif()
set (HAVE_GLOBALACCEL ${KF5GlobalAccel_FOUND})
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054900)
add_definitions(-DTRANSLATION_DOMAIN=\"kxmlgui5\")
add_definitions(-DQT_NO_FOREACH)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()

# Subdirectories
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5XmlGui")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5XmlGui_QCH
        FILE KF5XmlGuiQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5XmlGuiQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

set(HAVE_DBUS FALSE)
if (TARGET Qt5::DBus)
  set(HAVE_DBUS TRUE)
endif()
configure_package_config_file(
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
