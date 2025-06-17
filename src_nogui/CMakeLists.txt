aux_source_directory(. SRC_NOGUI)

file(GLOB CORE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_DIR}/*)

include_directories(${CORE_DIR} .)

add_executable(dwrgFpsUnlocker_nogui
        ${SRC_NOGUI}
        ${CORE}
        ${RES_DIR}/.rc
)

target_compile_options(dwrgFpsUnlocker_nogui PRIVATE /source-charset:utf-8)