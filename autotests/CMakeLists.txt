
find_package(Qt5 5.2.0 CONFIG REQUIRED Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMAddTests)

ecm_add_tests(
   kactioncategorytest.cpp
   kactioncollectiontest.cpp
   kmainwindow_unittest.cpp
   ktoolbar_unittest.cpp
   kxmlgui_unittest.cpp
   NAME_PREFIX "kxmlgui-"
   LINK_LIBRARIES Qt5::Test KF5::I18n KF5::IconThemes KF5::XmlGui
)
