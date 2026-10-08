cmake_minimum_required(VERSION 3.25)
project(MigGodotExample LANGUAGES CXX)
set(MIG_GODOT_EXAMPLE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/..")
find_path(MIG_GODOT_INTEGRATION_DIR native/CMakeLists.txt
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../integrations/godot"
          "${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/godot"
    NO_DEFAULT_PATH REQUIRED)
add_subdirectory("${MIG_GODOT_INTEGRATION_DIR}/native" bridge)
