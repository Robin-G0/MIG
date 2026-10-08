if(TARGET mig-c)
    add_executable(mig-c-api-tests c_api_tests.cpp)
    target_link_libraries(mig-c-api-tests PRIVATE mig-c)
    if(WIN32)
        add_custom_command(TARGET mig-c-api-tests POST_BUILD COMMAND ${CMAKE_COMMAND} -E
            copy_if_different "$<TARGET_FILE:mig-c>" "$<TARGET_FILE_DIR:mig-c-api-tests>")
    endif()
    add_test(NAME c-api COMMAND mig-c-api-tests)
endif()
