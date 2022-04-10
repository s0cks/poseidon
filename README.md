# Poseidon

Generational Garbage Collector based on dart/v8's parallel scavenge algorithm
# Building

## Prerequisites

* CMake
* glog
* gflags
* gtest - For testing

## Building Using CMake
```bash
mkdir ./build && cd ./build
cmake -DCMAKE_BUILD_TYPE=Release # change to Debug for debug builds
cmake --build .
```
