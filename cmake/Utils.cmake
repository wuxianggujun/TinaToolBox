# 定义一个函数来复制 DLL
function(copy_dll target_name dll_path dll_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll_path}/${dll_name}"
            $<TARGET_FILE_DIR:${target_name}>)
    message(STATUS "${dll_name} will be copied from: ${dll_path}/${dll_name}")
endfunction()