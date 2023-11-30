# Telos 
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/88658146-38ca-4dc7-896f-4166f3aed94a" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/bd56954d-c437-493e-9231-f8ed43eeab45" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/974f9011-08a2-4f5e-8a65-1d8b1b1bab1b" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Telos/assets/95353936/17432b8f-b17f-4222-92f0-c97231afbb0a" width="auto" height="100"></img> 
<img src="https://github.com/Argyraspides/Telos/assets/95353936/2fd4e08b-a1af-4f22-936e-c82f46fe4a7c" width="auto" height="100"></img>  
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/6e619ad7-d610-431d-9c0f-76ac519d074b" width="auto" height="100"></img>  


This is the source code for a highly improved version of my previous physics engine, which will also be able to run on any modern browser through the Emscripten toolchain (compilation into WebAssembly).
It uses the SDL2 graphics library for graphics output and ImGui for the UI. Upon completion, you may use the engine on my website [here](https://gaugamela.me). It is still in the planning stages. For the moment, a bare-bones build toolkit and 
some boilerplate SDL2 code are provided if you wish to build from source.

## Prerequisites
- ### [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) compatible browser
  - Nearly all modern browsers support WebAssembly. If you want to check if your browser is supported see [this page](https://caniuse.com/wasm).
- ### [SDL2](https://www.libsdl.org/)
  - Already included in this repo. SDL2 is a C Library which provides a low-level interface for multimedia and input functionalities.
- ### [Emscripten](https://emscripten.org/index.html)
  - Already included in this repo. EMscripten is a toolchain used to compile C++ to JS and ultimately [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) (.wasm).
- ### [ImGui](https://github.com/ocornut/imgui)
  - Already included in this repo. ImGui is a GUI library for C++.
## Building & Running (Linux)
To build and run in a web browser, simply run in the ``lib`` folder:
```
./run_web.sh
```
To build and run locally, i.e. a normal OS window:
```
./run_local.sh
```

If you get a permission denied error for running any of the shell scripts, you can simply run:
```
sudo chmod u+x {path to script here.sh}
```
