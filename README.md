# Telos 
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/88658146-38ca-4dc7-896f-4166f3aed94a" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/bd56954d-c437-493e-9231-f8ed43eeab45" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/974f9011-08a2-4f5e-8a65-1d8b1b1bab1b" width="auto" height="100"></img>
<img src="https://github.com/Argyraspides/Sarissa/assets/95353936/6e619ad7-d610-431d-9c0f-76ac519d074b" width="auto" height="100"></img>  
<img src="https://github.com/Argyraspides/Telos/assets/95353936/17432b8f-b17f-4222-92f0-c97231afbb0a" width="auto" height="100"></img>  

This is the source code for a highly improved version of my previous physics engine, which will also be able to run on any modern browser through the Emscripten toolchain (compilation into WebAssembly).
It uses the SDL2 graphics library for graphics output. Upon completion, you may use the engine on my website [here](https://gaugamela.me). It is still in the planning stages. For the moment, a bare-bones build toolkit and 
some boilerplate SDL2 code are provided if you wish to build from source.

## Prerequisites
- ### [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) compatible browser
  - Nearly all modern browsers support WebAssembly. If you want to check if your browser is supported see [this page](https://caniuse.com/wasm).
- ### [SDL2](https://www.libsdl.org/)
  - C++ Library used to render graphics and obtain peripheral inputs.
- ### [Emscripten](https://emscripten.org/index.html)
  - Already included in this repo, used to compile C++ to JS and ultimately [WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly) (.wasm).

## Building & Running (Debian Linux)
### The instructions below are for Debian-based Linux systems. Windows and macOS instructions will be added soon.

To install the prerequisites:
```
cd build_tools
./prerequisites.sh
```
To build *and run* the application onto a browser:
```
./build_web.sh
```
To build *and run* the application locally (i.e. in an SDL2 application window):
```
./build_local.sh
```

If you get a permission denied error for running any of the shell scripts, you can simply run:
```
sudo chmod u+x {path to script here.sh}
```
