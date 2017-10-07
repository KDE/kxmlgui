
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMAddTests)

ecm_add_tests(
   kactioncategorytest.cpp
   kactioncollectiontest.cpp
   LINK_LIBRARIES Qt5::Test KF5::XmlGui
)
ecm_add_tests(
   kmainwindow_unittest.cpp
   ktoolbar_unittest.cpp
   kxmlgui_unittest.cpp
   GUI
   LINK_LIBRARIES Qt5::Test KF5::IconThemes KF5::XmlGui
)

set_tests_properties(ktoolbar_unittest PROPERTIES RUN_SERIAL TRUE) # it wipes out ~/.qttest/share
