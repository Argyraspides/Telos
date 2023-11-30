# Refer to https://emscripten.org/docs/getting_started/downloads.html 
cd ./emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Navigate to the build directory for our CMake outputs
cd ..
cd ..
mkdir build
cd ./build

# Enable compiling with emscripten
header_file="../include/BUILD_EMCC.h"
new_string="#define BUILD_EMCC 1"
sed -i "1s/.*/$new_string/" "$header_file"
echo "$header_file definition changed to: $new_string"

# Pass in argument to CMakeLists.txt, set to EMCC compiler
cmake -DMY_COMPILER=emcc ..
cmake --build .

# Grab template HTML file, copy to this directory
source_file="../out/Telos.html"
cp "$source_file" .

emrun Telos.html