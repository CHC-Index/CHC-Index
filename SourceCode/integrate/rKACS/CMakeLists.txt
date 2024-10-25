# Set the project name and language Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and language
project(rKACS_Query CXX)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_BUILD_TYPE Debug) Set the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -O3 -DNDEBUG -fopenmp")
set(PROJECT_SOURCE_DIR src)
# set(3rdLib_SOURCE ../3rdPartyLib)
set(3rdLib_SOURCE $ENV{HOME}/CNNS-Experiment/codes_setup/CMP_codes/3rdPartyLib)

set(SNAP6 ${3rdLib_SOURCE}/snap)
# Set the path of glib-core folder in SNAP source code files
set(GLIB ${SNAP6}/glib-core)

# Set the path of snap-core folder in SNAP source code files
set(SNAP_CORE ${SNAP6}/snap-core)

# Set the folder of phmap.h (the source code can be downloaded in
# https://github.com/greg7mdp/parallel-hashmap/)
set(PARALLEL_HASHMAP ${3rdLib_SOURCE}/parallel-hashmap/parallel_hashmap)

# Add the include directories
include_directories(${GLIB} ${SNAP_CORE} ${PARALLEL_HASHMAP})

message(${SNAP_CORE})
#find_library(snap PATHS ${SNAP_CORE} REQUIRED)

add_executable(
        rKACS_Query
        src/main.cpp
        src/Common.cpp
        src/Def.cpp
        src/DynamicClique.cpp
        src/DynamicTCTruss.cpp
        src/KRCore.cpp
        src/MaxMinWeightTruss.cpp
        src/TrussComputation.cpp)
#target_link_libraries(rKACS_Query ${SNAP_CORE}/Snap.o)
target_link_libraries(rKACS_Query /Users/august/Downloads/snap-master/snap-core/Snap.o)

# target_link_libraries(rKACS_Query ${SNAP_CORE}/libsnap.a)