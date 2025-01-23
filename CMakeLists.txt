cmake_minimum_required(VERSION 3.18...3.29 FATAL_ERROR)

# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TinaToolBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Qt设置
set(QT_PREFIX_PATH "D:\\Programs\\Qt\\6.8.0\\msvc2022_64")
set(CMAKE_PREFIX_PATH ${QT_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

# 设置 PDFium_DIR 指向 PDFiumConfig.cmake 所在目录
set(PDFium_DIR "${PROJECT_SOURCE_DIR}/dependencies/pdfium-win-x64" CACHE PATH "Path to PDFiumConfig.cmake")
list(APPEND CMAKE_PREFIX_PATH ${PDFium_DIR})

# 设置ANTLR4的编译选项
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR4_TAG "4.13.1")
set(ANTLR4_SHARED_LIBS OFF)
set(ANTLR4_RUNTIME_ONLY ON)
set(ANTLR_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/antlr4/antlr-4.13.2-complete.jar")

# 编译器选项
if (MSVC)
    add_compile_options(/MP /bigobj /EHsc)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif ()

include(CPM)
include(CrashpadConfig)
include(Utils)

# 查找依赖包
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core5Compat Network Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core5Compat Network Sql)
find_package(ZLIB REQUIRED)
find_package(ANTLR REQUIRED)
find_package(PDFium REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(antlr4-runtime CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

# ANTLR4设置
find_package(Java COMPONENTS Runtime REQUIRED)
include(UseJava)

# Add ANTLR4 ExcelScript grammar
antlr_target(ExcelScript grammar/ExcelScript.g4
        LEXER
        PARSER
        LISTENER
        VISITOR
        PACKAGE TinaToolBox
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated/excel_script)

set(PROJECT_SOURCES "")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

# 添加选项控制是否构建模板项目
option(BUILD_TEMPLATE "Build TTBTemplate project" ON)

# 确保ANTLR4的include目录被正确包含
include_directories(
        ${ANTLR4_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/generated/excel_script
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 收集源文件
file(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/resources/*.qrc)
file(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h*)
file(GLOB_RECURSE ALL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 生成资源文件
qt_add_resources(RESOURCES ${RC_FILES})

# Add generated ANTLR files to the project sources
list(APPEND PROJECT_SOURCES
        ${ANTLR_ExcelScript_CXX_OUTPUTS}
        ${HEADER_FILES}
        ${ALL_SOURCE_FILES}
        ${RC_FILES}
        ${UI_FILES}
        ${RESOURCES})

# 在主项目配置之前先添加依赖项目
add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies)

qt_add_executable(TinaToolBox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES})

configure_crashed(TinaToolBox)

target_include_directories(TinaToolBox PRIVATE
        ${QT_PREFIX_PATH}/include
        ${PROJECT_SOURCE_DIR}/include
        ${ANTLR_ExcelScript_OUTPUT_DIR}
        ${PDFium_INCLUDE_DIRS}
)

target_compile_features(TinaToolBox PRIVATE cxx_std_17)

if (MSVC)
    target_link_options(TinaToolBox PRIVATE "/VERBOSE:LIB")
endif ()

# 链接库
target_link_libraries(TinaToolBox PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core5Compat
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        pdfium
        antlr4_shared
        cpp-terminal
        Tesseract::libtesseract
        utf8cpp
        xlnt
        ZLIB::ZLIB
        OpenSSL::SSL
        OpenSSL::Crypto
)

target_link_libraries(TinaToolBox PRIVATE
        "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Arrow::arrow_static,Arrow::arrow_shared>"
        "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Parquet::parquet_static,Parquet::parquet_shared>"
)

# 编译选项
target_compile_options(TinaToolBox PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
        $<$<COMPILE_LANGUAGE:CXX>:/permissive->
)
#target_compile_definitions(TinaToolBox PRIVATE CMAKE_TOOLCHAIN_FILE="D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake")

set_target_properties(TinaToolBox PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TinaToolBox
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(TinaToolBox)

if (WIN32)
    # 复制 pdfium.dll
    copy_dll(TinaToolBox "${PDFium_DIR}/bin" "pdfium.dll")

endif ()

if (BUILD_TEMPLATE)
    # 在构建模板项目之前，确保资源目录存在
    file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/resources)

    add_subdirectory(template)

    # 添加一个自定义命令来复制模板文件
    add_custom_command(
            OUTPUT ${CMAKE_SOURCE_DIR}/resources/template.exe
            COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:TTBTemplate>
            ${CMAKE_SOURCE_DIR}/resources/template.exe
            DEPENDS TTBTemplate
            COMMENT "Copying template executable to resources directory"
    )

    # 添加一个自定义目标来确保模板文件被复制
    add_custom_target(copy_template ALL
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/template.exe
    )
endif ()

# 添加测试目录（如果存在）
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif ()
