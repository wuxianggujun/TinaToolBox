cmake_minimum_required(VERSION 3.18...3.29 FATAL_ERROR)

project(TinaToolBox VERSION 0.1 LANGUAGES CXX)

set(QT_PREFIX_PATH "D:\\Programs\\Qt\\6.8.0\\msvc2022_64")
set(CMAKE_PREFIX_PATH ${QT_PREFIX_PATH} ${CMAKE_PREFIX_PATH})

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/ui)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network SQL)

set(PROJECT_SOURCES "") 
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")

file(GLOB_RECURSE RC_FILES  ${PROJECT_SOURCE_DIR}/resource/*.rc)
file(GLOB_RECURSE UI_FILES  ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)


aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)
 
# 生成资源文件
#qt_add_resources(RESOURCES ${RESOURCES_DIR}/resource.qrc)

list(APPEND PROJECT_SOURCES ${HEADER_FILES} ${SOURCE_FILES} ${RC_FILES} ${UI_FILES} ${RESOURCES})

add_subdirectory(${PROJECT_SOURCE_DIR}/dependencies)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TinaToolBox
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES})
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TinaToolBox APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TinaToolBox SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TinaToolBox
            ${PROJECT_SOURCES}
        )
    endif()
endif()

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${RC_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${UI_FILES})
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${RESOURCES})


target_include_directories(TinaToolBox PRIVATE ${QT_PREFIX_PATH}/include)
target_include_directories(TinaToolBox PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(TinaToolBox PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Sql)
#target_link_libraries(TinaToolBox PRIVATE QXlsx::QXlsx)
target_link_libraries(TinaToolBox PRIVATE spdlog::spdlog)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TinaToolBox)
endif()
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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TinaToolBox)
endif()
