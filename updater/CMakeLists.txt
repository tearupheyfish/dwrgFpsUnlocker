#1. updater本身需不需要更新？
#2. 全量覆盖还是差分
#3. 如果差分，那么跨多版本更新如何合并？

# updatepack


#set(CMAKE_PREFIX_PATH "R:\\Tofu\\Dependencies\\vcpkg\\installed\\x64-windows-static")

add_compile_options(/source-charset:utf-8)

find_package(minizip-ng CONFIG REQUIRED)
link_libraries(MINIZIP::minizip-ng)

add_executable(updater updater.cpp)



###################