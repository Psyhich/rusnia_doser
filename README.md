# Rusnia DDoS'er
A multipurpose DDoS tool with automatic and manual modes written on C++.

## Goal
Create easy to customize and modify DoS attacker with such base features:
 - HTTP request flood
 - UDP flood
 - TCP flood
 - 1 more other example of DoS attack to demonstrate customization

## How to build
### Compiling from sources
To compile this project from sources you will need:
- C++ compiler
- CMake
- Conan
- Linux machine

These are steps for compilation:
```
git clone https://github.com/Psyhich/rusnia_doser.git &&
cd rusnia_doser &&
mkdir build && cd build &&
conan install ../ --build=missing &&
cmake .. -DCMAKE_BUILD_TYPE=Release &&
cmake --build ./ -j 8 -t rusnia_doser
```
