
find_package(Qt5 5.2.0 CONFIG REQUIRED Xml Test)

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMAddTests)

macro(XMLGUI_TESTS)
    foreach(_testname ${ARGN})
        ecm_add_test(${_testname}.cpp TEST_NAME kxmlgui-${_testname} LINK_LIBRARIES Qt5::Test KF5::I18n KF5::IconThemes KF5::XmlGui)
    endforeach(_testname)
endmacro()

xmlgui_tests(
   kactioncategorytest
   kactioncollectiontest
   kmainwindow_unittest
   ktoolbar_unittest
   kxmlgui_unittest
)
