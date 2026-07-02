cmake_minimum_required(VERSION 3.19)
project(Homework_grading_system LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(Homework_grading_system
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    src/core/apiclient.h
    src/core/apiclient.cpp
    logindialog.h logindialog.cpp logindialog.ui
    studentpanel.h studentpanel.cpp studentpanel.ui
    teacherpanel.h teacherpanel.cpp teacherpanel.ui
)

qt_add_resources(Homework_grading_system "resources"
    PREFIX "/"
    FILES style.qss
)

target_link_libraries(Homework_grading_system
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

include(GNUInstallDirs)

install(TARGETS Homework_grading_system
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Homework_grading_system
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
