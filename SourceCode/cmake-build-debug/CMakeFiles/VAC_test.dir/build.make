# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /tmp/tmp.5TJTqsORwm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.5TJTqsORwm/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/VAC_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/VAC_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/VAC_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/VAC_test.dir/flags.make

CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o: CMakeFiles/VAC_test.dir/flags.make
CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o: /tmp/tmp.5TJTqsORwm/tests_arg/VAC-Test.cpp
CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o: CMakeFiles/VAC_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o -MF CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o.d -o CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o -c /tmp/tmp.5TJTqsORwm/tests_arg/VAC-Test.cpp

CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.5TJTqsORwm/tests_arg/VAC-Test.cpp > CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.i

CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.5TJTqsORwm/tests_arg/VAC-Test.cpp -o CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.s

# Object files for target VAC_test
VAC_test_OBJECTS = \
"CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o"

# External object files for target VAC_test
VAC_test_EXTERNAL_OBJECTS =

VAC_test: CMakeFiles/VAC_test.dir/tests_arg/VAC-Test.cpp.o
VAC_test: CMakeFiles/VAC_test.dir/build.make
VAC_test: CMakeFiles/VAC_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable VAC_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/VAC_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/VAC_test.dir/build: VAC_test
.PHONY : CMakeFiles/VAC_test.dir/build

CMakeFiles/VAC_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/VAC_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/VAC_test.dir/clean

CMakeFiles/VAC_test.dir/depend:
	cd /tmp/tmp.5TJTqsORwm/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.5TJTqsORwm /tmp/tmp.5TJTqsORwm /tmp/tmp.5TJTqsORwm/cmake-build-debug /tmp/tmp.5TJTqsORwm/cmake-build-debug /tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles/VAC_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/VAC_test.dir/depend

