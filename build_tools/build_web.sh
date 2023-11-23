# Go to Emscripten SDK directory
cd ./emsdk
# Temporarily set up the environment variables for the EMCC compiler
source ./emsdk_env.sh
# Translate the main CPP file to javascript 
emcc ../../src/main.cpp -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -o ../../out/web_build/index.js --emrun

# Bypass CORS policies and open the HTML file
# (you can open index.html manually, although 
# most modern browsers block loading of certain 
# files in certain ways with CORS)
emrun ../../out/web_build/index.html
