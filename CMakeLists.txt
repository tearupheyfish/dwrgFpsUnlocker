cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)

set(APP_VERSION "3.2.3")  #版本号
option(BUILD_GUI "编译图形用户界面版本" ON)
option(BUILD_SINGLE "编译单个可执行文件 (Qt静态链接）" OFF)
option(BUILTIN_MT "依赖静态多线程库 (/MT)")
option(USE_LOG "打印日志" OFF)
option(PRE_RELEASE "预发布版本" OFF)

set(DEPLOY_DIR "${CMAKE_SOURCE_DIR}/deploy")  #部署目录 可以用-DDEPLOY_DIR覆盖
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/resource/macroes.h.in"
            "${CMAKE_CURRENT_BINARY_DIR}/generated/macroes.h" #  传递为宏
    )
    include_directories("${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(CMAKE_INSTALL_PREFIX ${DEPLOY_DIR})         #只是为了防止一个管理员权限装到PROGRA~

project(dwrgFpsUnlocker VERSION ${APP_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("utilities.cmake")

file(GLOB COMMON_DIR "common") #共有目录绝对路径
file(GLOB RES_DIR "resource")  #资源目录绝对路径

if(BUILD_GUI)
    if(NOT BUILD_SINGLE)
        add_subdirectory("updater")
    endif ()
    add_subdirectory("src_gui")
else()
    add_subdirectory("src_nogui")
endif()





