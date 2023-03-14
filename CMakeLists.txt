cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "C:/VST_make/VST_SDK/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()
  
project(airwindows_500_lunchbox
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "airwindows_500_lunchbox VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(airwindows_500_lunchbox
    source/version.h
    source/lunchboxcids.h
    source/lunchboxprocessor.h
    source/lunchboxprocessor.cpp
    source/lunchboxcontroller.h
    source/lunchboxcontroller.cpp
    source/lunchboxentry.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/vst/vst2wrapper/vst2wrapper.sdk.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(airwindows_500_lunchbox
        PRIVATE
            resource/lunchboxeditor.uidesc
    )
    target_link_libraries(airwindows_500_lunchbox
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(airwindows_500_lunchbox
        RESOURCES
            "resource/lunchboxeditor.uidesc"
    )
endif(SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots (airwindows_500_lunchbox
    RESOURCES
        resource/9B16F1C851FB52B3BD5826BA9E94BA87_snapshot.png
        resource/9B16F1C851FB52B3BD5826BA9E94BA87_snapshot_2.0x.png
)

target_link_libraries(airwindows_500_lunchbox
    PRIVATE
        sdk
)

smtg_target_configure_version_file(airwindows_500_lunchbox)

if(SMTG_MAC)
    smtg_target_set_bundle(airwindows_500_lunchbox
        BUNDLE_IDENTIFIER io.github.yg331
        COMPANY_NAME "yg331"
    )
    smtg_target_set_debug_executable(airwindows_500_lunchbox
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(airwindows_500_lunchbox PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT airwindows_500_lunchbox)

        smtg_target_set_debug_executable(airwindows_500_lunchbox
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)

if(SMTG_WIN)
    if(NOT MINGW)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif(NOT MINGW)
endif(SMTG_WIN)