# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files (x86)\JetBrains\CLion 2016.3.5\bin\cmake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files (x86)\JetBrains\CLion 2016.3.5\bin\cmake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\zihao\Documents\GitHub\gallifrey

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/gallifrey.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/gallifrey.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/gallifrey.dir/flags.make

CMakeFiles/gallifrey.dir/src/main.cpp.obj: CMakeFiles/gallifrey.dir/flags.make
CMakeFiles/gallifrey.dir/src/main.cpp.obj: CMakeFiles/gallifrey.dir/includes_CXX.rsp
CMakeFiles/gallifrey.dir/src/main.cpp.obj: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/gallifrey.dir/src/main.cpp.obj"
	C:\MinGW\bin\g++.exe   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles\gallifrey.dir\src\main.cpp.obj -c C:\Users\zihao\Documents\GitHub\gallifrey\src\main.cpp

CMakeFiles/gallifrey.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gallifrey.dir/src/main.cpp.i"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\zihao\Documents\GitHub\gallifrey\src\main.cpp > CMakeFiles\gallifrey.dir\src\main.cpp.i

CMakeFiles/gallifrey.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gallifrey.dir/src/main.cpp.s"
	C:\MinGW\bin\g++.exe  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\zihao\Documents\GitHub\gallifrey\src\main.cpp -o CMakeFiles\gallifrey.dir\src\main.cpp.s

CMakeFiles/gallifrey.dir/src/main.cpp.obj.requires:

.PHONY : CMakeFiles/gallifrey.dir/src/main.cpp.obj.requires

CMakeFiles/gallifrey.dir/src/main.cpp.obj.provides: CMakeFiles/gallifrey.dir/src/main.cpp.obj.requires
	$(MAKE) -f CMakeFiles\gallifrey.dir\build.make CMakeFiles/gallifrey.dir/src/main.cpp.obj.provides.build
.PHONY : CMakeFiles/gallifrey.dir/src/main.cpp.obj.provides

CMakeFiles/gallifrey.dir/src/main.cpp.obj.provides.build: CMakeFiles/gallifrey.dir/src/main.cpp.obj


# Object files for target gallifrey
gallifrey_OBJECTS = \
"CMakeFiles/gallifrey.dir/src/main.cpp.obj"

# External object files for target gallifrey
gallifrey_EXTERNAL_OBJECTS =

gallifrey.exe: CMakeFiles/gallifrey.dir/src/main.cpp.obj
gallifrey.exe: CMakeFiles/gallifrey.dir/build.make
gallifrey.exe: CMakeFiles/gallifrey.dir/linklibs.rsp
gallifrey.exe: CMakeFiles/gallifrey.dir/objects1.rsp
gallifrey.exe: CMakeFiles/gallifrey.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable gallifrey.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\gallifrey.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/gallifrey.dir/build: gallifrey.exe

.PHONY : CMakeFiles/gallifrey.dir/build

CMakeFiles/gallifrey.dir/requires: CMakeFiles/gallifrey.dir/src/main.cpp.obj.requires

.PHONY : CMakeFiles/gallifrey.dir/requires

CMakeFiles/gallifrey.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\gallifrey.dir\cmake_clean.cmake
.PHONY : CMakeFiles/gallifrey.dir/clean

CMakeFiles/gallifrey.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\zihao\Documents\GitHub\gallifrey C:\Users\zihao\Documents\GitHub\gallifrey C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug C:\Users\zihao\Documents\GitHub\gallifrey\cmake-build-debug\CMakeFiles\gallifrey.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/gallifrey.dir/depend

