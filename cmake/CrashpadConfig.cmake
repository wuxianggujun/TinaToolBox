# Vcpkg下配置Crashed
include(CMakeParseArguments)

function(configure_crashed TARGET_NAME)
    set(options "")
    set(oneValueArgs HANDLER_OUTPUT_DIR)
    set(multiValueArgs "")

    cmake_parse_arguments(CRASHPAD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # 设置默认输出目录
    if (NOT CRASHPAD_HANDLER_OUTPUT_DIR)
        set(CRASHPAD_HANDLER_OUTPUT_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
    endif ()

    # 根据平台设置 handler 名称
    if(WIN32)
        set(CRASHPAD_HANDLER "crashpad_handler.exe")
    else()
        set(CRASHPAD_HANDLER "crashpad_handler")
    endif()

    # 查找 Crashpad 包
    find_package(crashpad CONFIG REQUIRED)

    # 链接 Crashpad 库
    target_link_libraries(${TARGET_NAME}
            PRIVATE
            crashpad::crashpad
            Threads::Threads
    )

    # 查找 crashpad_handler 路径
    string(REPLACE "share/crashpad" "tools/crashpad" CRASHPAD_TOOLS_DIR ${crashpad_DIR})
    set(CRASHPAD_HANDLER_PATH "${CRASHPAD_TOOLS_DIR}/${CRASHPAD_HANDLER}")
    message(STATUS "Target: ${TARGET_NAME}")
    message(STATUS "Crashpad handler path: ${CRASHPAD_HANDLER_PATH}")
    message(STATUS "Crashpad handler output directory: ${CRASHPAD_HANDLER_OUTPUT_DIR}")

    # 复制 crashpad_handler 到输出目录
    add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CRASHPAD_HANDLER_PATH}"
            "${CRASHPAD_HANDLER_OUTPUT_DIR}/${CRASHPAD_HANDLER}"
            COMMENT "Copying crashpad_handler for ${TARGET_NAME}"
    )
    
endfunction()