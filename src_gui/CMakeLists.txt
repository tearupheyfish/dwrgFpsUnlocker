
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # 使用 /MT 或 /MTd

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    list(PREPEND CMAKE_PREFIX_PATH "R:/Tofu/Dependencies/Libs/Qt_6.8.3-shared-Debug_msvc2022_64_RP")
else()
    list(PREPEND CMAKE_PREFIX_PATH "R:/Tofu/Dependencies/Libs/Qt_6.8.3-shared-Release_msvc2022_64_RP")
endif ()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB SRC_GUI RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
list(FILTER SRC_GUI EXCLUDE REGEX "^CMakeLists\\.txt$")

file(GLOB CORE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CORE_DIR}/*)

include_directories(${CORE_DIR} .)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dwrgFpsUnlocker
            MANUAL_FINALIZATION
            ${SRC_GUI}
            ${CORE}
            ${RES_DIR}/.rc
    )
endif()

target_compile_definitions(dwrgFpsUnlocker PRIVATE QT_DYNAMIC)

target_link_libraries(dwrgFpsUnlocker PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(dwrgFpsUnlocker PROPERTIES
        ${BUNDLE_ID_OPTION}
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
)

########################## DEPLOY via INSTALL ###########################
set(DEPLOY_DIR ${CMAKE_SOURCE_DIR}/deploy)

if (NOT EXISTS ${DEPLOY_DIR})
    file(MAKE_DIRECTORY ${DEPLOY_DIR})
endif ()

set(EXECUTABLE_PATH "$<TARGET_FILE:dwrgFpsUnlocker>")

add_custom_command(TARGET dwrgFpsUnlocker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_PATH} ${DEPLOY_DIR}
        COMMENT "Copying executable to deploy directory"
)

set(DEPLOY_COMMAND "windeployqt \
--release --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler \
--no-translations --add-plugin-types  platforms \
--dir ${DEPLOY_DIR} ${EXECUTABLE_PATH}")

install(CODE "
    message(\"Running windeployqt6 for deployment...\")
    execute_process(
        COMMAND ${DEPLOY_COMMAND}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE deploy_result
        ERROR_VARIABLE deploy_error
    )
    if(NOT deploy_result EQUAL 0)
        message(FATAL_ERROR \"windeployqt6 command failed: \${deploy_error}\")
    endif()
")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dwrgFpsUnlocker)
endif()