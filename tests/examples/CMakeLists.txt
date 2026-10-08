add_executable(mig-example-runtime-tests example_runtime_tests.cpp)
target_compile_features(mig-example-runtime-tests PRIVATE cxx_std_20)
add_test(NAME example-runtime COMMAND mig-example-runtime-tests
    "${CMAKE_CURRENT_BINARY_DIR}/example-runtime")
set_tests_properties(example-runtime PROPERTIES ENVIRONMENT "MIG_RUNTIME=")
if(MIG_BUILD_FORMAT)
    add_executable(mig-example-motion-tests example_motion_tests.cpp)
    target_link_libraries(mig-example-motion-tests PRIVATE mig-format)
    add_test(NAME example-motion COMMAND mig-example-motion-tests
        "${CMAKE_SOURCE_DIR}/examples/common/raised-hands.json")
endif()
