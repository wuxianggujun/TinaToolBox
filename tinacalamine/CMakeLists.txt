# --- 构建类型 ---
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose)
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release --verbose)
    set(TARGET_DIR "release")
endif ()

# --- 跨平台编译 ---
if (DEFINED ENV{CARGO_BUILD_TARGET})
    set(TARGET_DIR "$ENV{CARGO_BUILD_TARGET}/${TARGET_DIR}")
endif()

# --- Rust 库相关变量 ---
set(RUST_LIB_NAME tinacalamine)
set(RUST_SRC_DIR "${PROJECT_SOURCE_DIR}/${RUST_LIB_NAME}")
set(RUST_LIB_DIR "${RUST_SRC_DIR}/target")
set(RUST_PART_LIB "${RUST_LIB_DIR}/${TARGET_DIR}/${RUST_LIB_NAME}.lib")
set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/${RUST_LIB_NAME}.cpp")
set(RUST_PART_CXX_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${RUST_LIB_NAME}.h")

# 查找所有名为 lib.rs.cc 和 lib.rs.h 的文件
# 使用更精确的路径匹配模式, cxx在0.7版本后不再创建include 和 sources 文件夹
file(GLOB_RECURSE RUST_GENERATED_CC "${RUST_LIB_DIR}/${TARGET_DIR}/build/${RUST_LIB_NAME}-*/out/cxxbridge/sources/${RUST_LIB_NAME}/src/lib.rs.cc")
file(GLOB_RECURSE RUST_GENERATED_H "${RUST_LIB_DIR}/${TARGET_DIR}/build/${RUST_LIB_NAME}-*/out/cxxbridge/include/${RUST_LIB_NAME}/src/lib.rs.h")

# --- 自定义命令，构建 Rust 库并生成 C++ 桥接代码 ---
add_custom_command(
        OUTPUT ${RUST_PART_CXX} ${RUST_PART_CXX_HEADER} ${RUST_PART_LIB}
        COMMAND ${CMAKE_COMMAND} -E env CARGO_TARGET_DIR=${RUST_LIB_DIR} ${CARGO_CMD}
        COMMAND ${CMAKE_COMMAND} -E copy ${RUST_GENERATED_CC} ${RUST_PART_CXX}
        COMMAND ${CMAKE_COMMAND} -E copy ${RUST_GENERATED_H} ${RUST_PART_CXX_HEADER}
        COMMAND ${CMAKE_COMMAND} -E copy ${RUST_LIB_DIR}/${TARGET_DIR}/${RUST_LIB_NAME}.lib ${RUST_PART_LIB}
        WORKING_DIRECTORY ${RUST_SRC_DIR}
        COMMENT "Building Rust library and generating C++ bridge code"
        VERBATIM
)

# --- 添加静态库目标 ---
add_library(${RUST_LIB_NAME} STATIC ${RUST_PART_CXX})

# --- 添加头文件目录 ---
target_include_directories(${RUST_LIB_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        "${RUST_SRC_DIR}/include"
        "${RUST_SRC_DIR}/target/cxxbridge" # 使用更通用的路径
        ${CMAKE_CURRENT_BINARY_DIR}
)

# --- 链接库 ---
if(WIN32)
    target_link_libraries(${RUST_LIB_NAME} ${RUST_PART_LIB})
else()
    target_link_libraries(${RUST_LIB_NAME} pthread dl ${RUST_PART_LIB})
endif()

# --- 编译选项 (根据需要添加) ---
if(MSVC)
    target_compile_options(${RUST_LIB_NAME} PRIVATE /W4 /WX)
    target_link_options(${RUST_LIB_NAME} PRIVATE /WX)
endif()

# --- 添加测试 ---
add_test(NAME ${RUST_LIB_NAME}_test
        COMMAND cargo test
        WORKING_DIRECTORY ${RUST_SRC_DIR})