# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.30

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe"

# The command to remove a file.
RM = "D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

# Utility rule file for Demo9_autogen_timestamp_deps.

# Include any custom commands dependencies for this target.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\compiler_depend.make

# Include the progress variables for this target.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\progress.make

Demo9_autogen_timestamp_deps: dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\build.make
.PHONY : Demo9_autogen_timestamp_deps

# Rule to build all files generated by this target.
dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\build: Demo9_autogen_timestamp_deps
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\build

dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\clean:
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	$(CMAKE_COMMAND) -P CMakeFiles\Demo9_autogen_timestamp_deps.dir\cmake_clean.cmake
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\clean

dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo9_autogen_timestamp_deps.dir\depend

