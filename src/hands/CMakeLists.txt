add_library(mig-hands STATIC src/addon.cpp src/fingers.cpp)
add_library(MIG::hands ALIAS mig-hands)
set_target_properties(mig-hands PROPERTIES EXPORT_NAME hands)

target_compile_features(mig-hands PUBLIC cxx_std_20)
target_include_directories(mig-hands
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(mig-hands PUBLIC mig-core)

if(MIG_INSTALL)
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
