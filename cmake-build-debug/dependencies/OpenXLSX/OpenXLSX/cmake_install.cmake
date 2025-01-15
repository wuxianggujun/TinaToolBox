# Install script for directory: C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX

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

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX/headers" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/OpenXLSX/OpenXLSX-Exports.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX/headers" TYPE FILE FILES
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/IZipArchive.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCell.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCellIterator.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCellRange.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCellReference.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCellValue.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLColor.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLColumn.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLCommandQuery.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLComments.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLConstants.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLContentTypes.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLDateTime.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLDocument.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLException.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLFormula.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLIterator.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLMergeCells.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLProperties.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLRelationships.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLRow.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLRowData.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLSharedStrings.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLSheet.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLStyles.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLWorkbook.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLXmlData.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLXmlFile.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLXmlParser.hpp"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/headers/XLZipArchive.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenXLSX" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/OpenXLSX.hpp")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "lib" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/output/OpenXLSXd.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/OpenXLSX/OpenXLSX/OpenXLSXConfig.cmake"
    "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/OpenXLSX/OpenXLSX/OpenXLSXConfigVersion.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake"
         "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX/OpenXLSXTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/OpenXLSX" TYPE FILE FILES "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/OpenXLSX/CMakeFiles/Export/c72cc94553a1a0c9b05f75dae42fb1d7/OpenXLSXTargets-debug.cmake")
  endif()
endif()

