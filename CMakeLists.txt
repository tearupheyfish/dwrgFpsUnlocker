cmake_minimum_required(VERSION 3.19)

set(APP_VERSION "2.4")  #版本号
option(GUI_BUILD_SINGLE "编译单个可执行文件（静态链接）" OFF)
option(USE_LOG "打印日志" OFF)
option(PRE_RELEASE "预发布版本" OFF)
set(DEPLOY_DIR "${CMAKE_SOURCE_DIR}/deploy")  #部署目录 可以用-DDEPLOY_DIR覆盖

    configure_file(
            "${CMAKE_SOURCE_DIR}/resource/macroes.h.in"
            "${CMAKE_SOURCE_DIR}/common/inc/macroes.h" #  传递为宏
    )
    set(CMAKE_INSTALL_PREFIX ${DEPLOY_DIR})         #只是为了防止一个管理员权限装到PROGRA~


project(dwrgFpsUnlocker VERSION ${APP_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("utilities.cmake")

file(GLOB COMMON_DIR "common") #共有目录绝对路径
file(GLOB RES_DIR "resource")  #资源目录绝对路径

add_subdirectory("updater")
add_subdirectory("src_gui")
add_subdirectory("src_nogui")





