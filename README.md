# conways-game-of-life-cpp
Conway's game of life implementation on c++ with sfml and sfml-imgui

## build
Requirements: [conan](https://conan.io/downloads.html), [cmake >= 3.0](https://cmake.org/download/)
<br>
I assume you have already download this repo on your computer.
<br>
In project directory:
```
mkdir build
cd build
conan install .. --build missing
cmake -G <generator> -DCMAKE_BUILD_TYPE=Release ..
```
As generator you can use "Visual Studio 16" for Windows or "Unix Makefiles"
