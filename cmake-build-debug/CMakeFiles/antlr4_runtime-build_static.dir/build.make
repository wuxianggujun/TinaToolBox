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

# Utility rule file for antlr4_runtime-build_static.

# Include any custom commands dependencies for this target.
include CMakeFiles\antlr4_runtime-build_static.dir\compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles\antlr4_runtime-build_static.dir\progress.make

CMakeFiles\antlr4_runtime-build_static: antlr4_runtime\src\antlr4_runtime-stamp\antlr4_runtime-build_static

antlr4_runtime\src\antlr4_runtime-stamp\antlr4_runtime-build_static:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Performing build_static step for 'antlr4_runtime'"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\antlr4_runtime\src\antlr4_runtime\runtime\Cpp
	$(MAKE) antlr4_static
	echo >nul && "D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe" -E touch C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/antlr4_runtime/src/antlr4_runtime-stamp/antlr4_runtime-build_static
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

antlr4_runtime-build_static: CMakeFiles\antlr4_runtime-build_static
antlr4_runtime-build_static: antlr4_runtime\src\antlr4_runtime-stamp\antlr4_runtime-build_static
antlr4_runtime-build_static: CMakeFiles\antlr4_runtime-build_static.dir\build.make
.PHONY : antlr4_runtime-build_static

# Rule to build all files generated by this target.
CMakeFiles\antlr4_runtime-build_static.dir\build: antlr4_runtime-build_static
.PHONY : CMakeFiles\antlr4_runtime-build_static.dir\build

CMakeFiles\antlr4_runtime-build_static.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\antlr4_runtime-build_static.dir\cmake_clean.cmake
.PHONY : CMakeFiles\antlr4_runtime-build_static.dir\clean

CMakeFiles\antlr4_runtime-build_static.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles\antlr4_runtime-build_static.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles\antlr4_runtime-build_static.dir\depend

