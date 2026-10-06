if((MIG_BUILD_CONFIGURATOR OR MIG_BUILD_CONTROLLER) AND
   NOT EXISTS "${MIG_NATIVE_DEPS}/models/pose_landmarker_lite.task")
    message(FATAL_ERROR "Lite pose model missing: rerun tools/bootstrap-native.ps1")
endif()
foreach(app IN ITEMS configurator controller)
    string(TOUPPER "${app}" APP_UPPER)
    if(NOT MIG_BUILD_${APP_UPPER})
        continue()
    endif()
    set(app_sources main.cpp platform.cpp runtime.cpp capture.cpp inference.cpp
        keyboard.cpp controls.cpp diagnostics.cpp theme.cpp scrollbars.cpp)
    if(app STREQUAL "configurator")
        list(APPEND app_sources window.cpp renderer.cpp key_editor.cpp editor.cpp
            editor_navigation.cpp editor_settings.cpp editor_constraints.cpp)
    else()
        list(APPEND app_sources controller_window.cpp controller_profiles.cpp controller_preview.cpp
            controller_diagnostics.cpp)
    endif()
    add_executable(mig-${app} WIN32 ${app_sources})
    if(app STREQUAL "configurator")
        target_compile_definitions(mig-${app} PRIVATE MIG_CONFIGURATOR)
    endif()
    target_link_libraries(mig-${app} PRIVATE mig-native mig-format comdlg32 gdi32 user32 dwmapi shell32 comctl32 uxtheme)
    if(app STREQUAL "controller")
        target_link_libraries(mig-${app} PRIVATE mig-controller-profiles)
    endif()
    target_compile_options(mig-${app} PRIVATE /W4 /utf-8)
    set_target_properties(mig-${app} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin")
    add_custom_command(TARGET mig-${app} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MIG_NATIVE_DEPS}/mediapipe-package/mediapipe/tasks/c/libmediapipe.dll"
            "$<TARGET_FILE_DIR:mig-${app}>/libmediapipe.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:mig-${app}>/models" "$<TARGET_FILE_DIR:mig-${app}>/configs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MIG_NATIVE_DEPS}/models/pose_landmarker_full.task" "$<TARGET_FILE_DIR:mig-${app}>/models/pose_landmarker_full.task"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MIG_NATIVE_DEPS}/models/pose_landmarker_lite.task" "$<TARGET_FILE_DIR:mig-${app}>/models/pose_landmarker_lite.task"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/configs/default.json" "$<TARGET_FILE_DIR:mig-${app}>/configs/default.json")
    add_custom_target(run-${app}
        COMMAND "$<TARGET_FILE:mig-${app}>"
        DEPENDS mig-${app} USES_TERMINAL)
    if(MIG_BUILD_HANDS)
        add_custom_command(TARGET mig-${app} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MIG_NATIVE_DEPS}/models/hand_landmarker.task"
                "$<TARGET_FILE_DIR:mig-${app}>/models/hand_landmarker.task")
    endif()
    if(MIG_INSTALL)
        install(TARGETS mig-${app} RUNTIME DESTINATION bin)
    endif()
endforeach()
if(MIG_INSTALL)
    if(MIG_BUILD_HANDS)
        install(FILES "${MIG_NATIVE_DEPS}/models/hand_landmarker.task" DESTINATION bin/models)
    endif()
    install(FILES "${MIG_NATIVE_DEPS}/mediapipe-package/mediapipe/tasks/c/libmediapipe.dll" DESTINATION bin)
    install(FILES "${MIG_NATIVE_DEPS}/models/pose_landmarker_full.task" DESTINATION bin/models)
    install(FILES "${MIG_NATIVE_DEPS}/models/pose_landmarker_lite.task" DESTINATION bin/models)
    install(FILES "${CMAKE_SOURCE_DIR}/configs/default.json" DESTINATION bin/configs)
    install(FILES "${MIG_NATIVE_DEPS}/mediapipe-package/mediapipe-0.10.35.dist-info/licenses/LICENSE"
        DESTINATION share/mig/licenses RENAME MediaPipe-LICENSE)
    install(FILES "${MIG_NATIVE_DEPS}/nlohmann-LICENSE" DESTINATION share/mig/licenses)
endif()
