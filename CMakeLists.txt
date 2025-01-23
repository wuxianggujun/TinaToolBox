cmake_minimum_required(VERSION 3.18...3.29 FATAL_ERROR)

# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TinaToolBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf -8>")
#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(QT_PREFIX_PATH "D:\\Programs\\Qt\\6.8.0\\msvc2022_64")
#set(ARROW_PREFIX_PATH "D:/Programs/vcpkg/installed/x64-windows/share")
#set(CMAKE_PREFIX_PATH ${QT_PREFIX_PATH} ${CMAKE_PREFIX_PATH} ${ARROW_PREFIX_PATH})
set(CMAKE_PREFIX_PATH ${QT_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

set(PROJECT_SOURCES "")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

#include(ArrowConfig)
# 设置 PDFium_DIR 指向 PDFiumConfig.cmake 所在目录
set(PDFium_DIR "${PROJECT_SOURCE_DIR}/dependencies/pdfium-win-x64" CACHE PATH "Path to PDFiumConfig.cmake")

list(APPEND CMAKE_PREFIX_PATH ${PDFium_DIR})

if (MSVC)
    add_compile_options(/MP /bigobj /EHsc)
    # 禁用 Qt 关键字宏，包括 emit
    #    add_compile_definitions(QT_NO_KEYWORDS)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif ()

# 设置ANTLR4的编译选项
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR4_TAG "4.13.1")
set(ANTLR4_SHARED_LIBS OFF)
set(ANTLR4_RUNTIME_ONLY ON)

include(CPM)
include(CrashpadConfig)
include(Utils)

# 确保ANTLR4的include目录被正确包含
include_directories(
        ${ANTLR4_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}/generated/excel_script
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(ANTLR_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/antlr4/antlr-4.13.2-complete.jar")

CPMAddPackage(
        NAME Corrosion
        URL "${PROJECT_SOURCE_DIR}/dependencies/corrosion/corrosion-master.zip"
)
find_package(ZLIB REQUIRED)
find_package(ANTLR REQUIRED)
find_package(PDFium REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(antlr4-runtime CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat Widgets Network SQL)

# Add ANTLR4 ExcelScript grammar
antlr_target(ExcelScript grammar/ExcelScript.g4
        LEXER
        PARSER
        LISTENER
        VISITOR
        PACKAGE TinaToolBox
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated/excel_script)

# 确保生成的文件被包含
include_directories(${ANTLR_ExcelScript_OUTPUT_DIR})

file(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/resources/*.qrc)
file(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h*)

# 收集所有源文件，但排除TTBPacker
file(GLOB_RECURSE ALL_SOURCE_FILES 
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# 从源文件列表中排除TTBPacker.cpp
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX ".*TTBPacker\\.cpp$")

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

add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies)

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TinaToolBox
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES})
else ()
    if (ANDROID)
        add_library(TinaToolBox SHARED
                ${PROJECT_SOURCES}
        )
    else ()
        add_executable(TinaToolBox
                ${PROJECT_SOURCES}
        )
    endif ()
endif ()

configure_crashed(TinaToolBox)

target_include_directories(TinaToolBox PRIVATE
        ${QT_PREFIX_PATH}/include
        ${PROJECT_SOURCE_DIR}/include
        ${PDFium_INCLUDE_DIRS}
)

target_compile_features(TinaToolBox PRIVATE cxx_std_17)

if (MSVC)
    target_link_options(TinaToolBox PRIVATE "/VERBOSE:LIB")
endif ()

target_link_libraries(TinaToolBox PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core5Compat
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql)

target_link_libraries(TinaToolBox PRIVATE
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        pdfium
        antlr4_shared
        cpp-terminal
        Tesseract::libtesseract
        utf8cpp
        xlnt
        ZLIB::ZLIB
)

target_link_libraries(TinaToolBox PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(TinaToolBox PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Parquet::parquet_static,Parquet::parquet_shared>")
target_link_libraries(TinaToolBox PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Arrow::arrow_static,Arrow::arrow_shared>")


target_compile_options(TinaToolBox PRIVATE
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
        $<$<COMPILE_LANGUAGE:C>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
        $<$<COMPILE_LANGUAGE:CXX>:/permissive->
        $<$<COMPILE_LANGUAGE:CUDA>:-DNDEBUG>
)


target_compile_definitions(TinaToolBox PRIVATE CMAKE_TOOLCHAIN_FILE="D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake")


if (${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TinaToolBox)
endif ()
set_target_properties(TinaToolBox PROPERTIES
        ${BUNDLE_ID_OPTION}
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

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TinaToolBox)
endif ()

if (WIN32)
    # 复制 pdfium.dll
    copy_dll(TinaToolBox "${PDFium_DIR}/bin" "pdfium.dll")

    # 复制 xlnt 的 DLL
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(XLNT_DLL_NAME "xlntd.dll")
    else ()
        set(XLNT_DLL_NAME "xlnt.dll")
    endif ()
    copy_dll(TinaToolBox "${CMAKE_BINARY_DIR}/dependencies/xlnt/source" "${XLNT_DLL_NAME}")
endif ()

# 添加模板项目
add_subdirectory(template)

# 添加TTBPacker可执行文件
add_executable(TTBPacker
    src/TTBPacker.cpp
)

# 设置TTBPacker的包含目录
target_include_directories(TTBPacker PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接TTBPacker的依赖
if(MSVC)
    # 查找Windows SDK路径
    get_filename_component(WINSDK_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]" ABSOLUTE CACHE)
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(WINSDK_ARCH "x64")
    else()
        set(WINSDK_ARCH "x86")
    endif()
    
    # 添加Windows SDK库路径
    target_link_directories(TTBPacker PRIVATE
        "${WINSDK_DIR}/Lib/10.0.22621.0/um/${WINSDK_ARCH}"
    )
    
    target_link_libraries(TTBPacker PRIVATE 
        ZLIB::ZLIB
        imagehlp.lib
    )
    
    target_compile_options(TTBPacker PRIVATE /utf-8)
    target_compile_definitions(TTBPacker PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
endif()

# 创建resources目录（如果不存在）
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/resources/template.exe
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:TTBTemplate>
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/template.exe
    COMMAND ${CMAKE_COMMAND} -E echo "Copying template from: $<TARGET_FILE:TTBTemplate>"
    COMMAND ${CMAKE_COMMAND} -E echo "Copying template to: ${CMAKE_CURRENT_SOURCE_DIR}/resources/template.exe"
    DEPENDS TTBTemplate
)

# 添加一个自定义目标来确保模板文件被复制
add_custom_target(copy_template ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/resources/template.exe
)

# 添加依赖关系
add_dependencies(TTBPacker copy_template)

# 添加测试目录（如果存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    add_subdirectory(tests)
endif()
