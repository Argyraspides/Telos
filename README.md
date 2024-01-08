# Telos 
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/88658146-38ca-4dc7-896f-4166f3aed94a" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/bd56954d-c437-493e-9231-f8ed43eeab45" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/974f9011-08a2-4f5e-8a65-1d8b1b1bab1b" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Telos/assets/95353936/17432b8f-b17f-4222-92f0-c97231afbb0a" width="auto" height="100"></img> 
<img src="https://github.com/Argyraspides/Telos/assets/95353936/2fd4e08b-a1af-4f22-936e-c82f46fe4a7c" width="auto" height="100"></img>  
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/6e619ad7-d610-431d-9c0f-76ac519d074b" width="auto" height="100"></img>  


This is the source code for a highly improved version of my previous rigid body physics engine, which is also able to run on any modern browser through the Emscripten toolchain (compilation into WebAssembly).
It uses the SDL2 graphics library for graphics output and ImGui for the UI. Upon completion, you may use the engine in its full glory on my website [here](https://gaugamela.me/telos). The engine is currently on the site
in an unfinished state, if you'd like to check it out.

If you are using VSCode to check out the project, I have included the .vscode folder to take care of IntelliSense and all necessary include directories for both the emcc and gcc compiler for you :)

In the near future, a fluid dynamics and particle simulator module will be added as well.

## Prerequisites
- ### [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) compatible browser
  - Nearly all modern browsers support WebAssembly. If you want to check if your browser is supported see [this page](https://caniuse.com/wasm).
- ### [SDL2](https://www.libsdl.org/)
  - Already included in this repo. SDL2 is a C Library which provides a low-level interface for multimedia and input functionalities.
- ### [Emscripten](https://emscripten.org/index.html)
  - Already included in this repo. EMscripten is a toolchain used to compile C++ to JS and ultimately [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) (.wasm).
- ### [ImGui](https://github.com/ocornut/imgui)
  - Already included in this repo. ImGui is a GUI library for C++.
- ### [CMake](https://cmake.org/)
  - A powerful build tool for building C/C++ programs. You must install this yourself.
## Building & Running (Debian Linux)
First ensure you have OpenGL, CMake and essential build tools installed by running:
```
sudo apt-get update && sudo apt-get install build-essential
sudo apt install cmake
sudo apt install freeglut3-dev
```
To build and run in a web browser, simply run in the ``lib`` folder:
```
./run_web.sh
```
To build and run locally, i.e. a normal OS window:
```
./release_local.sh
```
If you get a permission denied error for running any of the shell scripts, you can simply run:

```
sudo chmod u+x {SCRIPT NAME}.sh
```
## Current High-level Application Architecture (MVC pattern)
#### Multi-threaded support is enabled, with the model running in a separate thread from the view and controller.
![MVC Overall Architecture drawio](https://github.com/Argyraspides/Telos/assets/95353936/c0a076dd-010f-4cdf-93a9-d3f537143780)
