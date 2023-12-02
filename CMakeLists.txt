cmake_minimum_required(VERSION 3.10)
project(Telos)
set(CMAKE_CXX_STANDARD 17)
set(MY_COMPILER "default" CACHE STRING "Choose a compiler: 'emcc', or 'gcc'")

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" # Telos source and header files
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/*.cpp" # ImGui source and header files
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl2.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdlrenderer2.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdlrenderer2.cpp"
)

# SDL2 already has it's own CMakeLists.txt, so we don't need to worry about it. We can just
# Add its location.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")
# We add our imgui as a library
add_library(imgui STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui.cpp")

# All include directories for .h files
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2/include" # SDL2
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui" # ImGui
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
   # "${CMAKE_CURRENT_SOURCE_DIR}/lib/emsdk/upstream/emscripten/cache/sysroot/include" # Emscripten 
    "${CMAKE_CURRENT_SOURCE_DIR}/include" # Telos
)

add_executable(${PROJECT_NAME} ${SOURCES}) 
find_package(OpenGL REQUIRED)
if(MY_COMPILER STREQUAL "emcc")

    set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/lib/emsdk/upstream/emscripten/emcc")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s FULL_ES3")

    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")
    target_link_libraries(${PROJECT_NAME} "imgui")

elseif((MY_COMPILER STREQUAL "gcc")) 

    target_link_libraries(${PROJECT_NAME} "SDL2" "imgui" "OpenGL::GL")

endif()