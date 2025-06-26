cmake_minimum_required(VERSION 3.16)

project(dwrgFpsUnlocker VERSION 2.2 LANGUAGES CXX)
set(RELEASE_TYPE release)

configure_file(
        ${CMAKE_SOURCE_DIR}/resource/macroes.h.in
        ${CMAKE_SOURCE_DIR}/common/macroes.h
)

set(DEPLOY_DIR ${CMAKE_SOURCE_DIR}/deploy)          #部署目录
if (NOT EXISTS ${DEPLOY_DIR})
    file(MAKE_DIRECTORY ${DEPLOY_DIR})
endif ()
set(CMAKE_INSTALL_PREFIX ${DEPLOY_DIR})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#共有目录绝对路径
file(GLOB COMMON_DIR common)
#资源目录绝对路径
file(GLOB RES_DIR resource)

#更新器
add_subdirectory(updater)
#修复器
#add_subdirectory(tryfix)

add_subdirectory(src_nogui)
add_subdirectory(src_gui)







