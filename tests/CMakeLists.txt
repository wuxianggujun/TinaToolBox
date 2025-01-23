# tests/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TinaToolBoxTests VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

set(PROJECT_SOURCES "")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

find_package(Arrow CONFIG REQUIRED) # 如果需要 Arrow，则保留
find_package(Parquet CONFIG REQUIRED) # 如果需要 Parquet，则保留

find_package(GTest REQUIRED)

# --- 手动指定头文件 ---
set(HEADER_FILES
        "${PROJECT_SOURCE_DIR}/../include/ThreadPool.hpp"
)

# 收集测试相关的源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src TEST_SOURCE_FILES)
message(STATUS "TEST_SOURCE_FILES: ${TEST_SOURCE_FILES}")

# --- 手动指定需要测试的源文件 ---
set(TESTABLE_SRC_FILES
        "${PROJECT_SOURCE_DIR}/../src/ThreadPool.cpp"
)

## 从 TESTABLE_SRC_FILES 中移除不想要测试的源文件
#list(REMOVE_ITEM TESTABLE_SRC_FILES
#        "${PROJECT_SOURCE_DIR}/../src/main.cpp"
#     
#)

list(APPEND PROJECT_SOURCES ${HEADER_FILES} ${TEST_SOURCE_FILES} ${TESTABLE_SRC_FILES})

#add_subdirectory(${PROJECT_SOURCE_DIR}/../dependencies/OpenXLSX ${CMAKE_CURRENT_BINARY_DIR}/OpenXLSX-build)
# 添加测试可执行文件
add_executable(TinaToolBoxTests ${PROJECT_SOURCES})

# 添加头文件搜索路径
target_include_directories(TinaToolBoxTests PRIVATE
        ${PROJECT_SOURCE_DIR}/../include
        ${GTEST_INCLUDE_DIRS} # GTest 头文件
)

target_compile_features(TinaToolBoxTests PRIVATE cxx_std_17)

# --- 编译选项 ---
if (MSVC)
    target_link_options(TinaToolBoxTests PRIVATE "/VERBOSE:LIB")
    target_compile_options(TinaToolBoxTests PRIVATE
            $<$<CONFIG:Debug>:/MDd>
            $<$<CONFIG:Release>:/MD>
    )
else()
    target_compile_options(TinaToolBoxTests PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-pthread> # 非 MSVC 编译器通常需要 -pthread 选项
    )
endif()

target_link_libraries(TinaToolBoxTests PRIVATE
        GTest::gtest
        GTest::gtest_main # 链接 gtest_main 库，它提供了 main 函数
        $<$<BOOL:${ARROW_BUILD_STATIC}>:Parquet::parquet_static>
        $<$<NOT:$<BOOL:${ARROW_BUILD_STATIC}>>:Parquet::parquet_shared>
        $<$<BOOL:${ARROW_BUILD_STATIC}>:Arrow::arrow_static>
        $<$<NOT:$<BOOL:${ARROW_BUILD_STATIC}>>:Arrow::arrow_shared>
)
target_compile_options(TinaToolBoxTests PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<COMPILE_LANGUAGE:C>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
        $<$<COMPILE_LANGUAGE:CXX>:/permissive->
        $<$<COMPILE_LANGUAGE:CUDA>:-DNDEBUG>
)

target_compile_definitions(TinaToolBoxTests PRIVATE CMAKE_TOOLCHAIN_FILE="D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake")

include(GNUInstallDirs)

# --- 使用 GTest 的 enable_testing 和 add_test ---
enable_testing()
add_test(NAME TinaToolBoxTests COMMAND $<TARGET_FILE:TinaToolBoxTests>)
