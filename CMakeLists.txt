cmake_minimum_required(VERSION 3.18...3.29 FATAL_ERROR)

# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TinaToolBox VERSION 0.1 LANGUAGES CXX)

set(QT_PREFIX_PATH "D:\\Programs\\Qt\\6.8.0\\msvc2022_64")
set(CMAKE_PREFIX_PATH ${QT_PREFIX_PATH} ${CMAKE_PREFIX_PATH})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

# 设置 PDFium_DIR 指向 PDFiumConfig.cmake 所在目录
set(PDFium_DIR "${PROJECT_SOURCE_DIR}/dependencies/pdfium-win-x64" CACHE PATH "Path to PDFiumConfig.cmake")

list(APPEND CMAKE_PREFIX_PATH ${PDFium_DIR})

find_package(PDFium REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat Widgets Network SQL)

set(PROJECT_SOURCES "")

file(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/resources/*.qrc)
file(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h*)

# 创建可执行文件
if (Qt${QT_VERSION_MAJOR}_USE_STATIC_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

# 生成资源文件
qt_add_resources(RESOURCES ${RC_FILES})

list(APPEND PROJECT_SOURCES ${HEADER_FILES} ${SOURCE_FILES} ${RC_FILES} ${UI_FILES} ${RESOURCES})

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

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${RC_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${UI_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${RESOURCES})

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
find_package(unofficial-skia CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG)

message(STATUS "Python Include_dirs: ${Python3_INCLUDE_DIRS}")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

target_include_directories(TinaToolBox PRIVATE
        ${QT_PREFIX_PATH}/include
        ${SIMPLEINI_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PDFium_INCLUDE_DIRS}
        ${Python3_INCLUDE_DIRS})

target_compile_features(TinaToolBox PRIVATE cxx_std_17)

target_link_libraries(TinaToolBox PRIVATE 
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core5Compat
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql)

target_link_libraries(TinaToolBox PRIVATE
        Python3::Python
        spdlog::spdlog
        QXlsx::QXlsx
        tomlplusplus::tomlplusplus
        unofficial::skia::skia
        pdfium)

target_compile_options(TinaToolBox PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:-DNDEBUG>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
        $<$<COMPILE_LANGUAGE:CXX>:/permissive->
        $<$<COMPILE_LANGUAGE:CXX>:/utf-8>
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
    add_custom_command(TARGET TinaToolBox POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PDFium_DIR}/bin/pdfium.dll"
            $<TARGET_FILE_DIR:TinaToolBox>)
endif()
