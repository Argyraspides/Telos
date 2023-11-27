#!/bin/bash
# The above line specifies that this is a Bash script

# Change to the emsdk directory
cd ./emsdk

# Source the emsdk environment script to set up the necessary variables
source ./emsdk_env.sh

# Compile the main.cpp source file using emcc (Emscripten Compiler)
# -s WASM=1: Enable WebAssembly output
# -s USE_SDL=2: Enable SDL2 support
# -s USE_SDL_IMAGE=2: Enable SDL2_image support
# -o ../../out/web_build/index.js: Output the compiled JavaScript to the specified path

header_file="../../src/BUILD_EMCC.h"
new_string="#define BUILD_EMCC 1"
sed -i "1s/.*/$new_string/" "$header_file"
echo "$header_file definition changed to: $new_string"

emcc ../../src/main.cpp -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -o ../../out/web_build/index.js --emrun

# Use emrun to serve the HTML file and run the compiled JavaScript
emrun ../../out/web_build/index.html
