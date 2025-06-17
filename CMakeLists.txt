cmake_minimum_required(VERSION 3.16)

project(dwrgFpsUnlocker VERSION 1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB CORE_DIR core)
file(GLOB RES_DIR resource)

add_subdirectory(src_gui)
add_subdirectory(src_nogui)






