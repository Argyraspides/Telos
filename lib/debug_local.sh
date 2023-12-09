# Refer to https://emscripten.org/docs/getting_started/downloads.html 
cd ./emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
emcc --version
cd ..
cd ..
mkdir debug
cd ./debug

# Enable compiling with emscripten
header_file="../include/BUILD_EMCC.h"
new_string="#define BUILD_EMCC 0"
sed -i "1s/.*/$new_string/" "$header_file"
echo "$header_file definition changed to: $new_string"

cmake -DMY_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .