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

# Include any dependencies generated for this target.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\depend.make
# Include any dependencies generated by the compiler for this target.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\compiler_depend.make

# Include the progress variables for this target.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\progress.make

# Include the compile flags for this target's objects.
include dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\flags.make

dependencies\OpenXLSX\Examples\Demo5_autogen\timestamp: D:\Programs\Qt\6.8.0\msvc2022_64\bin\moc.exe
dependencies\OpenXLSX\Examples\Demo5_autogen\timestamp: D:\Programs\Qt\6.8.0\msvc2022_64\bin\uic.exe
dependencies\OpenXLSX\Examples\Demo5_autogen\timestamp: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC and UIC for target Demo5"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	echo >nul && "D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe" -E cmake_autogen C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/Examples/CMakeFiles/Demo5_autogen.dir/AutogenInfo.json Debug
	echo >nul && "D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe" -E touch C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/dependencies/OpenXLSX/Examples/Demo5_autogen/timestamp
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\flags.make
dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj: dependencies\OpenXLSX\Examples\Demo5_autogen\mocs_compilation.cpp
dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object dependencies/OpenXLSX/Examples/CMakeFiles/Demo5.dir/Demo5_autogen/mocs_compilation.cpp.obj"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	$(CMAKE_COMMAND) -E cmake_cl_compile_depends --dep-file=CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj.d --working-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples --filter-prefix="注意: 包含文件:  " -- D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /showIncludes /FoCMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj /FdCMakeFiles\Demo5.dir\ /FS -c C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples\Demo5_autogen\mocs_compilation.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Demo5.dir/Demo5_autogen/mocs_compilation.cpp.i"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe > CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples\Demo5_autogen\mocs_compilation.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Demo5.dir/Demo5_autogen/mocs_compilation.cpp.s"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.s /c C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples\Demo5_autogen\mocs_compilation.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.obj: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\flags.make
dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.obj: C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples\Demo5.cpp
dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.obj: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object dependencies/OpenXLSX/Examples/CMakeFiles/Demo5.dir/Demo5.cpp.obj"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	$(CMAKE_COMMAND) -E cmake_cl_compile_depends --dep-file=CMakeFiles\Demo5.dir\Demo5.cpp.obj.d --working-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples --filter-prefix="注意: 包含文件:  " -- D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /showIncludes /FoCMakeFiles\Demo5.dir\Demo5.cpp.obj /FdCMakeFiles\Demo5.dir\ /FS -c C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples\Demo5.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/Demo5.dir/Demo5.cpp.i"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe > CMakeFiles\Demo5.dir\Demo5.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples\Demo5.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/Demo5.dir/Demo5.cpp.s"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\cl.exe @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\Demo5.dir\Demo5.cpp.s /c C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples\Demo5.cpp
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

# Object files for target Demo5
Demo5_OBJECTS = \
"CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj" \
"CMakeFiles\Demo5.dir\Demo5.cpp.obj"

# External object files for target Demo5
Demo5_EXTERNAL_OBJECTS =

output\Demo5.exe: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5_autogen\mocs_compilation.cpp.obj
output\Demo5.exe: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\Demo5.cpp.obj
output\Demo5.exe: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\build.make
output\Demo5.exe: output\OpenXLSXd.lib
output\Demo5.exe: dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable ..\..\..\output\Demo5.exe"
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	"D:\Program Files\JetBrains\Toolbox\CLion\bin\cmake\win\x64\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\Demo5.dir --rc=C:\PROGRA~2\WI3CF2~1\10\bin\100226~1.0\x86\rc.exe --mt=C:\PROGRA~2\WI3CF2~1\10\bin\100226~1.0\x86\mt.exe --manifests -- D:\PROGRA~2\MICROS~1\2022\PROFES~1\VC\Tools\MSVC\1442~1.344\bin\Hostx86\x64\link.exe /nologo @CMakeFiles\Demo5.dir\objects1.rsp @<<
 /out:..\..\..\output\Demo5.exe /implib:..\..\..\output\Demo5.lib /pdb:C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\output\Demo5.pdb /version:0.0 /machine:x64 /debug /INCREMENTAL /subsystem:console  ..\..\..\output\OpenXLSXd.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe -noprofile -executionpolicy Bypass -file D:/Programs/vcpkg/scripts/buildsystems/msbuild/applocal.ps1 -targetBinary C:/Users/wuxianggujun/CodeSpace/CMakeProjects/TinaToolBox/cmake-build-debug/output/Demo5.exe -installedDir D:/Programs/vcpkg/installed/x64-windows/debug/bin -OutVariable out
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug

# Rule to build all files generated by this target.
dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\build: output\Demo5.exe
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\build

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\clean:
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples
	$(CMAKE_COMMAND) -P CMakeFiles\Demo5.dir\cmake_clean.cmake
	cd C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\clean

dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\depend: dependencies\OpenXLSX\Examples\Demo5_autogen\timestamp
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\dependencies\OpenXLSX\Examples C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples C:\Users\wuxianggujun\CodeSpace\CMakeProjects\TinaToolBox\cmake-build-debug\dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : dependencies\OpenXLSX\Examples\CMakeFiles\Demo5.dir\depend

