# Build WASM version of Fotowall using emscripten

## Usage

From the root of this repository

```sh
devpod up . --id qt6-emscripten --devcontainer-path ./.devcontainer/qt6-emscripten/devcontainer.json --recreate
```

Then you can ssh into the container

```
ssh qt6-emscripten.devpod
```

Create a build directory and build the WASM version of the project

```sh
mkdir build-wasm
cd build-wasm
qt-cmake .. -G Ninja
ninja
```

You can test it using

```sh
python3 -m http.server
```

And open your browser to http://localhost:8000/fotowall.html
