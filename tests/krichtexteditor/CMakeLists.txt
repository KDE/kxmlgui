set(krichtexteditorSources
  main.cpp
  krichtexteditor.cpp
)

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/..
   ${CMAKE_CURRENT_BINARY_DIR}/..
   )
add_executable(krichtexteditor ${krichtexteditorSources} )
ecm_mark_as_test(krichtexteditor)
target_link_libraries(krichtexteditor Qt5::Test KF5::ConfigWidgets KF5::TextWidgets KF5::XmlGui)

#install(TARGETS krichtexteditor DESTINATION ${KDE_INSTALL_BINDIR})
#install(FILES krichtexteditorui.rc DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/krichtexteditor)

