cmake_minimum_required(VERSION 3.16)

# General configuration
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

project(sdl3-rmlui-template)

if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)    # Parallelize compilation in MSVC when Ninja is not used.
    endif()
endif()

set(EXECUTABLE_NAME ${PROJECT_NAME})

# Create platform-specific executable.
if (ANDROID)
    set(EXECUTABLE_NAME main)
    add_library(${EXECUTABLE_NAME} SHARED)
else()
    add_executable(${EXECUTABLE_NAME})
endif()

# *** Automatically load source files ***
file(GLOB_RECURSE SOURCES src/*.cpp) # Search for all .cpp files in src and subfolders.

# If we are on Windows, include the resource file for the icon
if(WIN32)
    set(ICON_RESOURCE src/resources.rc)
    target_sources(${EXECUTABLE_NAME} PRIVATE ${ICON_RESOURCE})
endif()

target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})

# *** Include directories ***
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/src  # Include all .h files in src and subfolders.
)

# Configuration for Apple: Specific files and resources.
if(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "src/logo.png")
endif()

# C++ standard requirements
target_compile_features(${EXECUTABLE_NAME} PUBLIC cxx_std_23)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# Use binaries or compile from scratch
option(USE_SDL_BINARIES "Use precompiled SDL3 and related libs" OFF)
# SDL3 conditional integration: binaries vs source
set(SDL3_BINARIES_PATH "../vendor/SDL3-3.2.16" CACHE PATH "Path to SDL3 precompiled binaries (should contain cmake/SDL3Config.cmake)")

if(USE_SDL_BINARIES)
    if(NOT SDL3_BINARIES_PATH)
        message(FATAL_ERROR "USE_SDL_BINARIES=ON but SDL3_BINARIES_PATH was not defined")
    endif()

    list(APPEND CMAKE_PREFIX_PATH "${SDL3_BINARIES_PATH}")
    find_package(SDL3 CONFIG REQUIRED)

    message(STATUS "Using SDL3 binary from: ${SDL3_BINARIES_PATH}")
else()
    message(STATUS "Compiling SDL3 from source")
    add_subdirectory(external/SDL EXCLUDE_FROM_ALL)
endif()

# Use binaries or compile from scratch
option(USE_SDL_IMAGE_BINARIES "Use precompiled SDL3_image and related libs" OFF)
# SDL3_image conditional integration: binaries vs source
set(SDL_IMAGE_BINARIES_PATH "../vendor/SDL3_image-3.2.4" CACHE PATH "Path to SDL3 precompiled binaries (should contain cmake/SDL3Config.cmake)")

if(USE_SDL_IMAGE_BINARIES)
    if(NOT SDL_IMAGE_BINARIES_PATH)
        message(FATAL_ERROR "USE_SDL_IMAGE_BINARIES=ON but SDL_IMAGE_BINARIES_PATH was not defined")
    endif()

    list(APPEND CMAKE_PREFIX_PATH "${SDL_IMAGE_BINARIES_PATH}")
    find_package(SDL3_image CONFIG REQUIRED)

    message(STATUS "Using SDL3_image binary from: ${SDL_IMAGE_BINARIES_PATH}")
else()
    message(STATUS "Compiling SDL3_image from source")
    set(SDLIMAGE_VENDORED ON)
    set(SDLIMAGE_AVIF OFF)
    set(SDLIMAGE_BMP OFF)
    set(SDLIMAGE_JPEG OFF)
    set(SDLIMAGE_WEBP OFF)
    add_subdirectory(external/SDL_image EXCLUDE_FROM_ALL)
endif()
# set(SDLTTF_VENDORED ON)
# add_subdirectory(SDL_ttf EXCLUDE_FROM_ALL)

# FreeType from submodule
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
add_subdirectory(external/freetype EXCLUDE_FROM_ALL)

# Create alias if not defined
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

set(SDLMIXER_MIDI_NATIVE OFF)
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_VENDORED ON)
add_subdirectory(external/SDL_Mixer EXCLUDE_FROM_ALL)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE RMLUI_SDL_VERSION_MAJOR=3)
add_subdirectory(external/RmlUi EXCLUDE_FROM_ALL)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC 
    # SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    SDL3_image::SDL3_image
    SDL3::SDL3
    RmlUi::RmlUi
)

if(APPLE AND NOT BUILD_SHARED_LIBS)
    find_library(IO_LIB ImageIO REQUIRED)
    find_library(CS_LIB CoreServices REQUIRED)
    find_library(CT_LIB CoreText REQUIRED)
    find_library(CG_LIB CoreGraphics REQUIRED)
    find_library(CF_LIB CoreFoundation REQUIRED)
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${CF_LIB} ${CT_LIB} ${IO_LIB} ${CS_LIB} ${CG_LIB})
endif()

target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# *** Manage custom commands to copy assets ***
set(ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/resources)                  # Original assets folder.
set(ASSETS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)     # Destination folder in the build directory.

# Automatically copy the assets folder to the build directory.
add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${ASSETS_DIR} ${ASSETS_OUTPUT_DIR}  # Copy the assets folder to the output directory.
    COMMENT "Copying assets to build directory (${ASSETS_OUTPUT_DIR})."
)

# Specific configuration for different platforms.
if (EMSCRIPTEN)
    file(GLOB ASSET_FILES ${ASSETS_DIR}/*)  # Find all files in assets.
    foreach(ASSET_FILE IN LISTS ASSET_FILES)
        get_filename_component(FILENAME ${ASSET_FILE} NAME)
        target_link_libraries(${EXECUTABLE_NAME} PRIVATE "--preload-file ${ASSETS_DIR}/${FILENAME}")
    endforeach()
endif()

if(IOS)
    set(RESOURCE_FILES "src/iosLaunchScreen.storyboard;src/logo.png")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES 
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_BUNDLE_IDENTIFIER "com.asdrome.sdl3-template"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.asdrome.sdl3-template"
    RESOURCE "${RESOURCE_FILES}"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    install(TARGETS ${EXECUTABLE_NAME}
        BUNDLE DESTINATION ./install COMPONENT Runtime
        RUNTIME DESTINATION ./install/bin COMPONENT Runtime
    )
    set(DEP_DIR "${CMAKE_BINARY_DIR}")
    INSTALL(CODE 
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        " 
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()

if(MINGW)
    target_link_options(${EXECUTABLE_NAME} PRIVATE "-mwindows")
endif()