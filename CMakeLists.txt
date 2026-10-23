cmake_minimum_required(VERSION 3.20)

project(
    UAVPathPlanningSimulator
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(voxel_grid src/VoxelGrid.cpp)
target_include_directories(voxel_grid PUBLIC include)

add_executable(uav_simulator src/main.cpp)
target_link_libraries(uav_simulator PRIVATE voxel_grid)

include(CTest)
if(BUILD_TESTING)
    add_executable(voxel_grid_tests tests/VoxelGridTests.cpp)
    target_link_libraries(voxel_grid_tests PRIVATE voxel_grid)
    add_test(NAME voxel_grid_tests COMMAND voxel_grid_tests)
endif()
