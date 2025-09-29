#set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL) # 这就是cmake默认参数

# 转换COMMON_DIR为相对路径
file(RELATIVE_PATH COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${COMMON_DIR}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Network)
find_package(Qt6Keychain CONFIG REQUIRED)

include_directories("${COMMON_DIR}/inc" "./inc")

aux_include_directory("./inc" HEADERS)
aux_source_directory("./src" SOURCES)
aux_source_directory("${COMMON_DIR}/src" COMSRC)
# 在分离ui h的情况下并入ui文件
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "./ui")

qt_add_executable(dwrgFpsUnlocker
        MANUAL_FINALIZATION
        ${HEADERS} ${SOURCES}
        ${COMSRC} #comdir/inc
        ${RES_DIR}/.rc
)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_link_options(/OPT:REF /OPT:ICF)
endif()
target_link_libraries(dwrgFpsUnlocker PRIVATE
        Qt6::Widgets Qt6::Network Qt6Keychain::Qt6Keychain)

set_target_properties(dwrgFpsUnlocker PROPERTIES
        ${BUNDLE_ID_OPTION}
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
)


########################## DEPLOY via INSTALL ###########################
if(${GUI_BUILD_SINGLE})
    set(DEPLOY_DIR "${DEPLOY_DIR}/sgui")
else()
    set(DEPLOY_DIR "${DEPLOY_DIR}/gui")
endif()


installmsg("### Preparing deployment directory...")
install(CODE "
        file(REMOVE_RECURSE \"${DEPLOY_DIR}\")
        file(MAKE_DIRECTORY \"${DEPLOY_DIR}\")
")

installmsg("### Installing executable...")
install(TARGETS dwrgFpsUnlocker updater DESTINATION "${DEPLOY_DIR}")

if(NOT GUI_BUILD_SINGLE)
    set(DEPLOY_COMMAND "windeployqt \
    --release --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler \
    --no-translations --add-plugin-types  platforms \
    --dir ${DEPLOY_DIR} ${EXECUTABLE_PATH}")

    installmsg("Running windeployqt6 for deployment...")
    install(CODE "
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
endif ()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dwrgFpsUnlocker)
endif()