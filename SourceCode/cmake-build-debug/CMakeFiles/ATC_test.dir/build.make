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
include CMakeFiles/ATC_test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ATC_test.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ATC_test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ATC_test.dir/flags.make

CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o: CMakeFiles/ATC_test.dir/flags.make
CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o: /tmp/tmp.5TJTqsORwm/tests_arg/ATC-Test.cpp
CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o: CMakeFiles/ATC_test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o -MF CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o.d -o CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o -c /tmp/tmp.5TJTqsORwm/tests_arg/ATC-Test.cpp

CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /tmp/tmp.5TJTqsORwm/tests_arg/ATC-Test.cpp > CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.i

CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /tmp/tmp.5TJTqsORwm/tests_arg/ATC-Test.cpp -o CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.s

# Object files for target ATC_test
ATC_test_OBJECTS = \
"CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o"

# External object files for target ATC_test
ATC_test_EXTERNAL_OBJECTS =

ATC_test: CMakeFiles/ATC_test.dir/tests_arg/ATC-Test.cpp.o
ATC_test: CMakeFiles/ATC_test.dir/build.make
ATC_test: CMakeFiles/ATC_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ATC_test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ATC_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ATC_test.dir/build: ATC_test
.PHONY : CMakeFiles/ATC_test.dir/build

CMakeFiles/ATC_test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ATC_test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ATC_test.dir/clean

CMakeFiles/ATC_test.dir/depend:
	cd /tmp/tmp.5TJTqsORwm/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.5TJTqsORwm /tmp/tmp.5TJTqsORwm /tmp/tmp.5TJTqsORwm/cmake-build-debug /tmp/tmp.5TJTqsORwm/cmake-build-debug /tmp/tmp.5TJTqsORwm/cmake-build-debug/CMakeFiles/ATC_test.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/ATC_test.dir/depend

