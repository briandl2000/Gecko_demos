cmake_minimum_required(VERSION 3.22)
project(Breakout LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fill in the Gecko release tag you want to use. See:
#   https://github.com/briandl2000/Gecko/releases
set(GECKO_VERSION "0.0.0-alpha.3-dev.20260514122359")   # <-- change me
include(cmake/FetchGecko.cmake)

add_executable(Breakout src/main.cpp src/App.cpp)
target_link_libraries(Breakout PRIVATE
    Gecko::Core
    Gecko::CoreServices
    Gecko::Math
    Gecko::Runtime
    Gecko::Platform
    Gecko::Graphics     # remove if you don't need the renderer
)

# On Linux, the executable needs to look in its own directory for the
# Gecko .so files we copy next to it below.
if(UNIX AND NOT APPLE)
  set_target_properties(Breakout PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Copy Gecko's runtime shared library (CoreServices) next to the
# executable so it runs in-place on both Linux and Windows.
#
# Note: $<TARGET_RUNTIME_DLLS:...> is empty on non-DLL platforms (Linux
# and macOS), so it does NOT cover the libGeckoCoreServices.so case --
# use $<TARGET_FILE:...> against the shared imported target instead.
add_custom_command(TARGET Breakout POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:Gecko::CoreServices>"
        "$<TARGET_FILE_DIR:Breakout>"
    VERBATIM
)

# Copy compile_commands.json to the project root so clangd / your editor
# can find it without extra config.
# Note: This only works with generators that produce compile_commands.json (e.g., Ninja).
# Visual Studio generators don't generate this file.
if(CMAKE_EXPORT_COMPILE_COMMANDS AND NOT MSVC_IDE)
  add_custom_target(copy_compile_commands ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${PROJECT_SOURCE_DIR}/compile_commands.json"
        BYPRODUCTS "${PROJECT_SOURCE_DIR}/compile_commands.json"
        VERBATIM
    )
endif()

gecko_add_shaders(
  TARGET    Breakout
  NAMESPACE breakout
  HEADER    shaders.h
  SHADERS
    triangle.vert.hlsl=TriangleVert
    triangle.frag.hlsl=TriangleFrag
    fullscreen.vert.hlsl=FullscreenVert
    fullscreen.frag.hlsl=FullscreenFrag
    plasma.comp.hlsl=PlasmaComp
)
