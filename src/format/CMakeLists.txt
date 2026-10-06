add_library(mig-format STATIC src/schema.cpp src/configuration.cpp src/atomic_file.cpp src/json_model.cpp)
add_library(MIG::format ALIAS mig-format)
set_target_properties(mig-format PROPERTIES EXPORT_NAME format)

target_compile_features(mig-format PUBLIC cxx_std_20)
target_include_directories(mig-format
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(mig-format PUBLIC mig-core)
target_include_directories(mig-format PRIVATE
    "$<TARGET_PROPERTY:nlohmann_json::nlohmann_json,INTERFACE_INCLUDE_DIRECTORIES>")

if(MIG_INSTALL)
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
