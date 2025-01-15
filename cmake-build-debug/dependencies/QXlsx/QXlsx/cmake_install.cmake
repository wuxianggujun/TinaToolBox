# Install script for directory: C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/TinaToolBox")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/QXlsxQt6.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/QXlsxQt6" TYPE FILE FILES
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxabstractooxmlfile.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxabstractsheet.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxabstractsheet_p.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxcellformula.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxcell.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxcelllocation.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxcellrange.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxcellreference.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxchart.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxchartsheet.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxconditionalformatting.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxdatavalidation.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxdatetype.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxdocument.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxformat.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxglobal.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxrichstring.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxworkbook.h"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/QXlsx/QXlsx/header/xlsxworksheet.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/CMakeFiles/QXlsx.dir/install-cxx-module-bmi-Debug.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "devel" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6/QXlsxQt6Targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6/QXlsxQt6Targets.cmake"
         "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/CMakeFiles/Export/5e1a71f991ec0867fe453527b0963803/QXlsxQt6Targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6/QXlsxQt6Targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6/QXlsxQt6Targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/CMakeFiles/Export/5e1a71f991ec0867fe453527b0963803/QXlsxQt6Targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/CMakeFiles/Export/5e1a71f991ec0867fe453527b0963803/QXlsxQt6Targets-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/QXlsxQt6" TYPE FILE FILES
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/QXlsxQt6Config.cmake"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/QXlsx/QXlsx/QXlsxQt6ConfigVersion.cmake"
    )
endif()

