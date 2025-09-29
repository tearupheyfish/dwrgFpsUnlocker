macro(installmsg message_text)
    install(CODE "message(STATUS \"${message_text}\")")
endmacro()

function(aux_include_directory dir var)
    file(GLOB _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${dir}/*.h)
    set(${var} ${_files} PARENT_SCOPE)  # 使用 PARENT_SCOPE 明确设置外部变量
endfunction()

