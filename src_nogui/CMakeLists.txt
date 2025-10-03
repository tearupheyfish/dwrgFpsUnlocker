file(RELATIVE_PATH COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${COMMON_DIR}")

find_package(yalantinglibs CONFIG REQUIRED)

include_directories("./finc" "${COMMON_DIR}/inc" "./inc" )

aux_source_directory("./src" SRC_NOGUI)
aux_source_directory("${COMMON_DIR}/src" SRC_COMMON)

add_executable(dwrgFpsUnlocker_nogui
        ${SRC_NOGUI} ${SRC_COMMON}
)

target_compile_options(dwrgFpsUnlocker_nogui PRIVATE /utf-8)
target_compile_definitions(dwrgFpsUnlocker_nogui PRIVATE UNICODE _UNICODE)

target_link_libraries(dwrgFpsUnlocker_nogui PRIVATE yalantinglibs::yalantinglibs)
set_target_properties(
        dwrgFpsUnlocker_nogui PROPERTIES
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE"
)


########################## DEPLOY via INSTALL ###########################

set(DEPLOY_DIR ${DEPLOY_DIR}/nogui)

add_custom_command(TARGET dwrgFpsUnlocker_nogui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DEPLOY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory   ${DEPLOY_DIR}
)

install(TARGETS dwrgFpsUnlocker_nogui
        RUNTIME DESTINATION ${DEPLOY_DIR}
)

