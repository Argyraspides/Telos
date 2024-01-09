cmake_minimum_required(VERSION 3.22)
project(Telos)
set(CMAKE_CXX_STANDARD 17)
set(MY_COMPILER "default" CACHE STRING "Choose a compiler: 'emcc', or 'gcc'")

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" # Telos source and header files
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/*.cpp" # ImGui source and header files.
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl2.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdl2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdlrenderer2.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends/imgui_impl_sdlrenderer2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2_gfx-1.0.4/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2_gfx-1.0.4/*.cpp"

)

# SDL2 already has it's own CMakeLists.txt, so we don't need to worry about it. We can just
# Add its location.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")

# We add our imgui as a library
add_library(imgui STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/imgui.cpp")


# Add our gfx library (not native to SDL2).
file(GLOB SDL2GFX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2_gfx-1.0.4/*.cpp")
add_library(SDL2GFX STATIC ${SDL2GFX_SOURCES})


# All include directories for .h files
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2/include" # SDL2
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui" # ImGui
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/backends"
   # "${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui/examples/libs/emscripten/emscripten_mainloop_stub.h"
   # "${CMAKE_CURRENT_SOURCE_DIR}/lib/emsdk/upstream/emscripten/cache/sysroot/include" # Emscripten 
    "${CMAKE_CURRENT_SOURCE_DIR}/include" # Telos
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2_gfx-1.0.4" 
)

#OpenGL is literally so common surely i dont need to add this myself lmao.
add_executable(${PROJECT_NAME} ${SOURCES}) 
find_package(OpenGL REQUIRED)

# If we are building to run on a web browser, we need to use the EMCC compiler.
if(MY_COMPILER STREQUAL "emcc")

    # Set the compiler. This is bundled in the emscripten SDK.
    set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/lib/emsdk/upstream/emscripten/emcc")
    # Emscripten is a C++-to-JS compiler, so we want our output to be a .js file and not a normal binary
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    # Emscripten is compatible with SDL2, so we use this flag to enable it ("-s USE_SDL=2")
    # For this particular build, the model is run in a separate thread from the view and controller,
    # so we enable multi-threaded support by passing in -pthread and the max number of threads we are allowed
    # to allocate (in this case as many threads as there are CPU cores)
    # Obviously, any files outside of what is included by the compiler, that would be accessible on our local machine (as they still exist
    # on the computer), will not be accessible by the web browser once we compile to web assembly. Any extra files that we want to access
    # must be preloaded in order for us to access them, hence --preload-file
    # -O2 is the compiler optimization. For the web, we want our WASM and JS files to be as small as possible, but also balance for 
    # performance, -O2 is a good tradeoff from -O3 for this reason 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s USE_SDL=2 -pthread -s PTHREAD_POOL_SIZE=navigator.hardwareConcurrency -s TOTAL_MEMORY=512MB --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets/Roboto@assets/Roboto -O3")

    link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2")
    target_link_libraries(${PROJECT_NAME} "imgui" "SDL2GFX")

elseif((MY_COMPILER STREQUAL "gcc")) 
    target_link_libraries(${PROJECT_NAME} "SDL2" "SDL2GFX" "imgui" "OpenGL::GL" )
endif() 