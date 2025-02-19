This docker image is used to build and publish the webassembly release of fotowall using Emscripten compiler.

Usage
###

First build the image using

```
./build.sh
```

Then run it using

```
./run.sh
```

This opens a bash shell in the container. From it you can run the normal cmake pipeline commands and it will cross-compile it using Emscripten.

```
cd build
cmake -GNinja ..
ninja
```
