cmake_minimum_required(VERSION 3.16)
project(CNNS)

set(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message ("OPENMP FOUND")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fopenmp")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${0penMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS" ${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

#set(CMAKE_BUILD_TYPE Release)
 set(CMAKE_BUILD_TYPE Debug)
include_directories(./include)
include_directories(./integrate)
include_directories(./3rdPartyLib)
include_directories(./3rdPartyLib/parallel_hashmap)

#add_executable(Whole tests/CHCI-Whole-Procedure.cpp)
add_executable(Build tests/CHCI-Build.cpp)
add_executable(Retrieve tests/CHCI-Retrieve-Multi.cpp)
add_executable(Retrieve_adjust tests/CHCI-Retrieve-L.cpp)
add_executable(ATC_test tests/ATC-Test.cpp)
add_executable(VAC_test tests/VAC-Test.cpp)

add_executable(Retrieve_Del tests/CHCI-Del-Retrieve-Multi.cpp)
add_executable(Part_Build tests/CHCI-Part_Build.cpp)
add_executable(Add_Nodes tests/CHCI-Part_Add.cpp)

set(3rdLib_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/3rdPartyLib)

#set(3rdLib_SOURCE /Users/august/NewStart/CNNS/3rdPartyLib)

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
# find_library(SNAP PATHS ${SNAP_CORE} REQUIRED)

add_executable(
       rKACS_Query
       ./integrate/rKACS/src/main.cpp
       ./integrate/rKACS/src/Common.cpp
       ./integrate/rKACS/src/Def.cpp
       ./integrate/rKACS/src/DynamicClique.cpp
       ./integrate/rKACS/src/DynamicTCTruss.cpp
       ./integrate/rKACS/src/KRCore.cpp
       ./integrate/rKACS/src/MaxMinWeightTruss.cpp
       ./integrate/rKACS/src/TrussComputation.cpp)
target_link_libraries(rKACS_Query ${SNAP_CORE}/Snap.o)

