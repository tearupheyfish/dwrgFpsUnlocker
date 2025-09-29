# updatepack

add_compile_options(/source-charset:utf-8)

find_package(minizip-ng CONFIG REQUIRED)
link_libraries(MINIZIP::minizip-ng)

add_executable(updater updater.cpp)

###################