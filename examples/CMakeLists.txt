cmake_minimum_required(VERSION 3.25)
project(MIGExamples LANGUAGES CXX)
include(common/sdk.cmake)
add_subdirectory(sdk-consumer)
add_subdirectory(native-consumer)
add_subdirectory(sdl2)
add_subdirectory(sfml)
include(CTest)
if(BUILD_TESTING AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../tests/example_profile_tests.cpp")
    add_executable(mig-example-profile-tests ../tests/example_profile_tests.cpp)
    target_link_libraries(mig-example-profile-tests PRIVATE MIG::core MIG::format MIG::hands)
    target_compile_features(mig-example-profile-tests PRIVATE cxx_std_20)
    target_compile_definitions(mig-example-profile-tests PRIVATE
        MIG_EXAMPLE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/.."
        MIG_EXAMPLE_COMMON="${CMAKE_CURRENT_SOURCE_DIR}/common")
    add_test(NAME example-profile-imports COMMAND mig-example-profile-tests
        "${CMAKE_CURRENT_SOURCE_DIR}/common/raised-hands.json")
endif()
