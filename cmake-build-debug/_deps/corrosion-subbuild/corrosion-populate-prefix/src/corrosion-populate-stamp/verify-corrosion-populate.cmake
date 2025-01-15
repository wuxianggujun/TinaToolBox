# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip")
  message(FATAL_ERROR "File not found: C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip")
endif()

if("" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip'")

file("" "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/corrosion/corrosion-master.zip
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
