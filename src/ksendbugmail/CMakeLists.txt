set(ksendbugmail_SRCS
  main.cpp
  smtp.cpp
)

add_executable(ksendbugmail ${ksendbugmail_SRCS})
ecm_mark_nongui_executable(ksendbugmail)

target_link_libraries(ksendbugmail
   Qt5::Widgets
   Qt5::Network
   KF5::ConfigCore # KEmailSettings
   KF5::I18n
)
if(WIN32)
    target_link_libraries(ksendbugmail secur32) # GetUserNameEx()
endif()

install(TARGETS ksendbugmail DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF5} )

