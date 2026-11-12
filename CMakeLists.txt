cmake_minimum_required(VERSION 3.22)

project(Pedalier VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- JUCE ------------------------------------------------------------------
# By default JUCE is downloaded at configure time. To use a local checkout
# instead (faster, works offline): cmake -DJUCE_DIR=/path/to/JUCE ..
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")

if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.9
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

# --- ASIO (Windows, optional) ----------------------------------------------
# WASAPI works out of the box. For the lowest latency with a Focusrite
# interface, point ASIO_SDK_DIR at the Steinberg ASIO SDK folder.
set(ASIO_SDK_DIR "" CACHE PATH "Path to the Steinberg ASIO SDK (Windows only, optional)")

# --- Test bench app (step 1) -------------------------------------------------
juce_add_gui_app(PedalierBancDEssai
    PRODUCT_NAME "Pedalier - Banc d'essai"
    COMPANY_NAME "Pedalier"
    VERSION ${PROJECT_VERSION})

target_sources(PedalierBancDEssai PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/PassThroughEngine.cpp
    Source/MidiMonitor.cpp)

target_compile_definitions(PedalierBancDEssai PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:PedalierBancDEssai,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:PedalierBancDEssai,JUCE_VERSION>")

if(WIN32 AND ASIO_SDK_DIR)
    target_compile_definitions(PedalierBancDEssai PRIVATE JUCE_ASIO=1)
    target_include_directories(PedalierBancDEssai PRIVATE "${ASIO_SDK_DIR}/common")
endif()

target_link_libraries(PedalierBancDEssai
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
