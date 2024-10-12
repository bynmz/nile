#!/bin/bash
mkdir -p build
cd build
cmake -DUSE_ASAN=OFF -S ../ -B .
make && make Shaders && ./NileEngine
cd ..