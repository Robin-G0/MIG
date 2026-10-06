add_library(mig-c SHARED api.cpp)
add_library(MIG::c ALIAS mig-c)
set_target_properties(mig-c PROPERTIES EXPORT_NAME c OUTPUT_NAME mig-c
    VERSION ${PROJECT_VERSION} SOVERSION 1
    CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
target_compile_definitions(mig-c PRIVATE MIG_C_EXPORTS)
target_include_directories(mig-c PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(mig-c PRIVATE mig-format)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(mig-c PRIVATE "LINKER:--exclude-libs,ALL")
endif()
if(TARGET mig-hands)
    target_link_libraries(mig-c PRIVATE mig-hands)
    target_compile_definitions(mig-c PRIVATE MIG_C_HANDS)
endif()
if(TARGET mig-native)
    target_link_libraries(mig-c PRIVATE mig-native)
    target_compile_definitions(mig-c PRIVATE MIG_C_NATIVE)
endif()
if(MIG_INSTALL)
    install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
