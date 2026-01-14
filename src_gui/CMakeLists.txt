# 转换COMMON_DIR为相对路径
file(RELATIVE_PATH COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${COMMON_DIR}")
file(RELATIVE_PATH RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}" "${RES_DIR}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (MSVC AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/Os /Oy /Ob0 /Gy)
    add_link_options(/OPT:REF /OPT:ICF /LTCG /INCREMENTAL:NO /MERGE:.rdata=.text)
endif ()

find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets Core Network)
check_qt_build_type(${BUILD_SINGLE})
find_package(QHotKey CONFIG REQUIRED)
add_compile_definitions(NOMINMAX)#windows.h的minmax宏和ylt打架
find_package(yalantinglibs CONFIG REQUIRED)

include_directories("${COMMON_DIR}/inc" "./inc")

aux_include_directory("./inc" HEADERS)
aux_source_directory("./src" SOURCES)
aux_source_directory("${COMMON_DIR}/src" COMSRC)
# 在分离ui h的情况下并入ui文件
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS "./ui")

qt6_add_executable(dwrgFpsUnlocker
        MANUAL_FINALIZATION
        ${HEADERS} ${SOURCES}
        ${COMSRC} #comdir/inc
        ${RES_DIR}/.rc
)
qt6_add_resources(dwrgFpsUnlocker
        PREFIX "/"
        BASE
            "${RES_DIR}"
        FILES
            "${RES_DIR}/纯彩mini.ico"
        OPTIONS
)

target_compile_definitions(dwrgFpsUnlocker PRIVATE
        $<$<BOOL:${BUILD_SINGLE}>:BUILD_SINGLE>
        $<$<BOOL:${USE_LOG}>:USE_LOG>
        $<$<BOOL:${PRE_RELEASE}>:PRE_RELEASE>
)
target_link_libraries(dwrgFpsUnlocker PRIVATE
        Qt6::Widgets Qt6::Network
        qhotkey
        yalantinglibs::yalantinglibs
        Wbemuuid
)

set_target_properties(dwrgFpsUnlocker PROPERTIES
        ${BUNDLE_ID_OPTION}
        WIN32_EXECUTABLE TRUE
        VERSION ${PROJECT_VERSION}
        LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
)


########################## DEPLOY via INSTALL ###########################
if(${BUILD_SINGLE})
    set(DEPLOY_DIR "${DEPLOY_DIR}/sgui")
else()
    set(DEPLOY_DIR "${DEPLOY_DIR}/gui")
endif()

if (MSVC AND NOT CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "DLL")
    string(APPEND DEPLOY_DIR "f")
endif ()


    installmsg("### Preparing deployment directory...")
    install(CODE "
            file(REMOVE_RECURSE \"${DEPLOY_DIR}\")
            file(MAKE_DIRECTORY \"${DEPLOY_DIR}\")
    ")

    installmsg("### Installing executable...")
    install(TARGETS dwrgFpsUnlocker DESTINATION "${DEPLOY_DIR}")

if(NOT BUILD_SINGLE)
    set(DEPLOY_COMMAND "windeployqt6 \
    --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler \
    --no-translations --add-plugin-types  platforms \
    --dir ${DEPLOY_DIR} $<TARGET_FILE:dwrgFpsUnlocker>")

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

    install(TARGETS updater DESTINATION "${DEPLOY_DIR}")
endif ()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dwrgFpsUnlocker)
endif()