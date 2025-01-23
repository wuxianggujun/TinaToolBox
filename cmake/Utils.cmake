# 定义一个函数来复制 DLL
function(copy_dll target_name dll_path dll_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll_path}/${dll_name}"
            $<TARGET_FILE_DIR:${target_name}>)
    message(STATUS "${dll_name} will be copied from: ${dll_path}/${dll_name}")
endfunction()

function(add_windows_resource target_name resource_file)
    if (WIN32)
        # Define resource file and output .res file
        set(RESOURCE_FILE "${resource_file}")
        set(RESOURCE_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${target_name}_resources.res") # 根据目标名生成资源文件名

        # Custom command to compile resources.rc using rc.exe
        add_custom_command(
                OUTPUT ${RESOURCE_OUTPUT_FILE}
                COMMAND rc.exe -fo ${RESOURCE_OUTPUT_FILE} ${RESOURCE_FILE}
                DEPENDS ${RESOURCE_FILE}
                COMMENT "Compiling resources: ${RESOURCE_FILE}"
                VERBATIM
        )

        # Custom target to ensure resource compilation happens before linking
        add_custom_target(CompileResources_${target_name} ALL # 为每个目标创建独立的资源编译目标
                DEPENDS ${RESOURCE_OUTPUT_FILE}
                COMMENT "Ensuring resource compilation for ${target_name}..."
        )

        # Link the generated resources.res file to your executable
        target_link_libraries(${target_name} PRIVATE ${RESOURCE_OUTPUT_FILE})
        add_dependencies(${target_name} CompileResources_${target_name}) # 依赖于特定目标的资源编译目标
    else()
        message(STATUS "Skipping resource compilation because it's not a Windows platform.")
    endif()
endfunction()