# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /snap/clion/145/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/145/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/msatskevich/Desktop/MIPT++/AllocList

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/AllocList.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/AllocList.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/AllocList.dir/flags.make

CMakeFiles/AllocList.dir/main.cpp.o: CMakeFiles/AllocList.dir/flags.make
CMakeFiles/AllocList.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/AllocList.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/AllocList.dir/main.cpp.o -c /home/msatskevich/Desktop/MIPT++/AllocList/main.cpp

CMakeFiles/AllocList.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/AllocList.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/msatskevich/Desktop/MIPT++/AllocList/main.cpp > CMakeFiles/AllocList.dir/main.cpp.i

CMakeFiles/AllocList.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/AllocList.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/msatskevich/Desktop/MIPT++/AllocList/main.cpp -o CMakeFiles/AllocList.dir/main.cpp.s

# Object files for target AllocList
AllocList_OBJECTS = \
"CMakeFiles/AllocList.dir/main.cpp.o"

# External object files for target AllocList
AllocList_EXTERNAL_OBJECTS =

AllocList: CMakeFiles/AllocList.dir/main.cpp.o
AllocList: CMakeFiles/AllocList.dir/build.make
AllocList: CMakeFiles/AllocList.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable AllocList"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/AllocList.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/AllocList.dir/build: AllocList

.PHONY : CMakeFiles/AllocList.dir/build

CMakeFiles/AllocList.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/AllocList.dir/cmake_clean.cmake
.PHONY : CMakeFiles/AllocList.dir/clean

CMakeFiles/AllocList.dir/depend:
	cd /home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/msatskevich/Desktop/MIPT++/AllocList /home/msatskevich/Desktop/MIPT++/AllocList /home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug /home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug /home/msatskevich/Desktop/MIPT++/AllocList/cmake-build-debug/CMakeFiles/AllocList.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/AllocList.dir/depend

