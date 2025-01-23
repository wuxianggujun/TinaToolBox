cmake_minimum_required(VERSION 3.15)
# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TTBTemplate)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

# 添加资源文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/resources.rc
        @ONLY)

# 设置源文件
set(TEMPLATE_SOURCES
        src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBFile.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBCrypto.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBScriptEngine.cpp
        ${CMAKE_SOURCE_DIR}/src/ExcelScriptInterpreter.cpp
        ${CMAKE_SOURCE_DIR}/src/ExcelHandler.cpp
        ${CMAKE_SOURCE_DIR}/src/LogSystem.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/resources.rc
)

# 添加可执行文件
add_executable(TTBTemplate ${TEMPLATE_SOURCES})

# 添加包含目录
target_include_directories(TTBTemplate PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
)

# 链接必要的库
target_link_libraries(TTBTemplate PRIVATE
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
)

# 设置编译选项
if (MSVC)
    target_compile_options(TTBTemplate PRIVATE /utf-8)
    target_compile_definitions(TTBTemplate PRIVATE
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif ()