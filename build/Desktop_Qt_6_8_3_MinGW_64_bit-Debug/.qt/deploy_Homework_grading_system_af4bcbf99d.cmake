include("E:/Homework_grading_system/build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/Homework_grading_system-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE E:/Homework_grading_system/build/Desktop_Qt_6_8_3_MinGW_64_bit-Debug/Homework_grading_system.exe
    GENERATE_QT_CONF
)
