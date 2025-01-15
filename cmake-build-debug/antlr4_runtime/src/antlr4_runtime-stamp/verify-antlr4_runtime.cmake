# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip")
  message(FATAL_ERROR "File not found: C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip")
endif()

if("SHA256" STREQUAL "")
  message(WARNING "File cannot be verified since no URL_HASH specified")
  return()
endif()

if("69a05dd5266cada443d5fae9cf38fc4784affb468d19e575955e3a17c185ebd6" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(VERBOSE "verifying file...
     file='C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip'")

file("SHA256" "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip" actual_value)

if(NOT "${actual_value}" STREQUAL "69a05dd5266cada443d5fae9cf38fc4784affb468d19e575955e3a17c185ebd6")
  message(FATAL_ERROR "error: SHA256 hash of
  C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/dependencies/antlr4/antlr4-dev.zip
does not match expected value
  expected: '69a05dd5266cada443d5fae9cf38fc4784affb468d19e575955e3a17c185ebd6'
    actual: '${actual_value}'
")
endif()

message(VERBOSE "verifying file... done")
