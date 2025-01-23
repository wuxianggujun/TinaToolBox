cmake_minimum_required(VERSION 3.15)
# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TTBTemplate)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置ANTLR4的编译选项
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR4_TAG "4.13.1")
set(ANTLR4_SHARED_LIBS OFF)
set(ANTLR4_RUNTIME_ONLY ON)

# 查找必要的包
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(antlr4-runtime CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# 创建资源目录
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources)

# 添加资源文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/resources/TTBTemplate.rc
        @ONLY)

message(STATUS "Resource file will be generated at: ${CMAKE_CURRENT_BINARY_DIR}/resources/TTBTemplate.rc")

# 设置源文件
set(TEMPLATE_SOURCES
        src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBFile.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBCrypto.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBPacker.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBScriptEngine.cpp
        ${CMAKE_SOURCE_DIR}/src/ExcelScriptInterpreter.cpp
        ${CMAKE_SOURCE_DIR}/src/ExcelHandler.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBResourceLoader.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBResourceHandle.cpp
        ${CMAKE_SOURCE_DIR}/src/TTBTemporaryFile.cpp
        ${ANTLR_ExcelScript_CXX_OUTPUTS}
)

# 添加可执行文件
add_executable(TTBTemplate ${TEMPLATE_SOURCES})

# 添加资源文件编译
add_windows_resource(TTBTemplate ${CMAKE_CURRENT_BINARY_DIR}/resources/TTBTemplate.rc)

# 添加包含目录
target_include_directories(TTBTemplate PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_BINARY_DIR}/generated/excel_script
        ${ANTLR4_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/dependencies/xlnt/source/../include
)

# 链接必要的库
target_link_libraries(TTBTemplate PRIVATE
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
        antlr4_shared
        xlnt
        spdlog::spdlog
)

# 设置编译选项
if (MSVC)
    target_compile_options(TTBTemplate PRIVATE /utf-8)
    target_compile_definitions(TTBTemplate PRIVATE
            _CRT_SECURE_NO_WARNINGS
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _WIN32_WINNT=0x0601
            _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
    )
endif()

# 添加安装规则
install(TARGETS TTBTemplate
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 打印输出路径信息
message(STATUS "TTBTemplate will be built to: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Resource file location: ${CMAKE_CURRENT_BINARY_DIR}/resources/TTBTemplate.rc")

# 确保在主项目的ANTLR文件生成后再构建模板
add_dependencies(TTBTemplate antlr4_shared)