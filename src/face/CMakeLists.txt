add_library(mig-face STATIC src/addon.cpp)
add_library(MIG::face ALIAS mig-face)
set_target_properties(mig-face PROPERTIES EXPORT_NAME face)

target_compile_features(mig-face PUBLIC cxx_std_20)
target_include_directories(mig-face
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(mig-face PUBLIC mig-core)

if(MIG_INSTALL)
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
