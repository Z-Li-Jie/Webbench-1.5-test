# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.12.2/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.12.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/zlj/Tests/general/webbench-1.5-test/webbench-test4

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build

# Include any dependencies generated for this target.
include CMakeFiles/webbench.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/webbench.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/webbench.dir/flags.make

CMakeFiles/webbench.dir/webbench.c.o: CMakeFiles/webbench.dir/flags.make
CMakeFiles/webbench.dir/webbench.c.o: ../webbench.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/webbench.dir/webbench.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/webbench.dir/webbench.c.o   -c /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/webbench.c

CMakeFiles/webbench.dir/webbench.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/webbench.dir/webbench.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/webbench.c > CMakeFiles/webbench.dir/webbench.c.i

CMakeFiles/webbench.dir/webbench.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/webbench.dir/webbench.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/webbench.c -o CMakeFiles/webbench.dir/webbench.c.s

# Object files for target webbench
webbench_OBJECTS = \
"CMakeFiles/webbench.dir/webbench.c.o"

# External object files for target webbench
webbench_EXTERNAL_OBJECTS =

webbench: CMakeFiles/webbench.dir/webbench.c.o
webbench: CMakeFiles/webbench.dir/build.make
webbench: CMakeFiles/webbench.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable webbench"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/webbench.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/webbench.dir/build: webbench

.PHONY : CMakeFiles/webbench.dir/build

CMakeFiles/webbench.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/webbench.dir/cmake_clean.cmake
.PHONY : CMakeFiles/webbench.dir/clean

CMakeFiles/webbench.dir/depend:
	cd /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zlj/Tests/general/webbench-1.5-test/webbench-test4 /home/zlj/Tests/general/webbench-1.5-test/webbench-test4 /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build /home/zlj/Tests/general/webbench-1.5-test/webbench-test4/build/CMakeFiles/webbench.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/webbench.dir/depend

