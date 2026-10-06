add_library(mig-core STATIC src/library.cpp src/engine.cpp src/geometry.cpp
    src/motion_validation.cpp src/calibration.cpp src/input.cpp src/constraints.cpp src/session.cpp)
add_library(MIG::core ALIAS mig-core)
set_target_properties(mig-core PROPERTIES EXPORT_NAME core)
target_compile_definitions(mig-core PRIVATE MIG_LIBRARY_VERSION="${PROJECT_VERSION}")

target_compile_features(mig-core PUBLIC cxx_std_20)
target_include_directories(mig-core
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(MIG_INSTALL)
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
