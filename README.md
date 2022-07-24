# Rusnia DDoS'er
A multipurpose DDoS tool with automatic and manual modes written on C++.

## How to build
### Docker
To build and run it on docker just run:
```
git clone https://github.com/Psyhich/rusnia_doser
cd rusnia_doser
docker build -t rusnia_doser:latest ./
```
And after image is built run:
```
docker run rusnia_doser:latest
```
### Compiling from sources
To compile this project from sources you will need:
- C++ compiler
- CMake
- Conan
- Linux machine

These are steps for compilation:
```
git clone --recurse-submodules https://github.com/Psyhich/rusnia_doser.git &&
cd rusnia_doser &&
mkdir build && cd build &&
conan install ../ &&
cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build ./ -j 8 -t rusnia_doser
```
