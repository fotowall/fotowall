#!/bin/bash

HOST_SRC=${PWD}/..
DOCKER_SRC="/src"
DOCKER_BUILD=$DOCKER_SRC/build
BUILD_CMD="mkdir -p build && cmake -B $DOCKER_BUILD -S $DOCKER_SRC ; cmake --build $DOCKER_BUILD"
echo $HOST_SRC
echo $DOCKER_SRC
echo $DOCKER_BUILD
echo $BUILD_CMD

docker run \
  -it \
  --rm \
  -v "$HOST_SRC:$DOCKER_SRC/" \
  -u $(id -u):$(id -g) \
  qt-webassembly-cmake-extended \
  bash
