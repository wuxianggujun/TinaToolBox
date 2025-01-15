# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-src")
  file(MAKE_DIRECTORY "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-src")
endif()
file(MAKE_DIRECTORY
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-build"
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix"
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/tmp"
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/src/apachearrow-populate-stamp"
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/src"
  "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/src/apachearrow-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/src/apachearrow-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/_deps/apachearrow-subbuild/apachearrow-populate-prefix/src/apachearrow-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
