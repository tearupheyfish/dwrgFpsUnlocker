cmake_minimum_required(VERSION 3.16)

project(dwrgFpsUnlocker VERSION 2.0 LANGUAGES CXX)
set(RELEASE_TYPE prerelease)

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

file(GLOB COMMON_DIR common)
file(GLOB RES_DIR resource)

add_subdirectory(updater)

add_subdirectory(src_nogui)
add_subdirectory(src_gui)







