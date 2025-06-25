include_directories(${COMMON_DIR})

add_compile_options(/source-charset:utf-8)

add_executable(tryfix tryfix.cpp ${COMMON_DIR}/winapiimpl.cpp)