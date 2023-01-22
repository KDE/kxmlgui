add_executable(ksendbugmail)

target_sources(ksendbugmail PRIVATE
    main.cpp
    smtp.cpp
)

ecm_mark_nongui_executable(ksendbugmail)

target_link_libraries(ksendbugmail
   Qt6::Widgets
   Qt6::Network
   KF6::ConfigCore # KEmailSettings
   KF6::I18n
)
if(WIN32)
    target_link_libraries(ksendbugmail secur32) # GetUserNameEx()
endif()

install(TARGETS ksendbugmail DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF} )

