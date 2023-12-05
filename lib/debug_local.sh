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


cmake -DMY_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .