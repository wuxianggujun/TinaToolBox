cmake_minimum_required(VERSION 3.18...3.29 FATAL_ERROR)

# 设置搜索路径优先级
# MSYS2 Configuration

set(MSYS2_PATH "D:、Programs/msys64" CACHE PATH "MSYS2 installation path")
list(APPEND CMAKE_PREFIX_PATH "${MSYS2_PATH}/mingw64")
list(APPEND CMAKE_LIBRARY_PATH "${MSYS2_PATH}/mingw64/lib")
list(APPEND CMAKE_INCLUDE_PATH "${MSYS2_PATH}/mingw64/include")

# vcpkg configuration
set(CMAKE_TOOLCHAIN_FILE "D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# 设置查找包的策略
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

project(TinaToolBox VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_QT6 REQUIRED poppler-qt6)
pkg_check_modules(POPPLER REQUIRED poppler)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network SQL)

set(PROJECT_SOURCES "")
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

file(GLOB_RECURSE RC_FILES ${PROJECT_SOURCE_DIR}/resource/*.rc)
file(GLOB_RECURSE UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

# 生成资源文件
#qt_add_resources(RESOURCES ${RESOURCES_DIR}/resource.qrc)

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
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
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

find_package(unofficial-skia CONFIG REQUIRED)

target_include_directories(TinaToolBox PRIVATE ${QT_PREFIX_PATH}/include)
target_include_directories(TinaToolBox PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_include_directories(TinaToolBox PRIVATE ${POPPLER_QT6_INCLUDE_DIRS})
target_link_libraries(TinaToolBox PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql)
#target_link_libraries(TinaToolBox PRIVATE QXlsx::QXlsx)
target_link_libraries(TinaToolBox PRIVATE spdlog::spdlog)
target_link_libraries(TinaToolBox PRIVATE ${POPPLER_LIBRARIES} ${POPPLER_QT6_LIBRARIES})
target_link_libraries(TinaToolBox PRIVATE unofficial::skia::skia)
target_compile_definitions(-DCMAKE_TOOLCHAIN_FILE=D:/Programs/vcpkg/scripts/buildsystems/vcpkg.cmake)
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
