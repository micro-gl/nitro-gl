<div align='center'>
<img src='nitro-gl-logo-rounded.png' style='height: 200px;'/>
</div>

# nitro{gl}
**Headers** only **`C++11`** **OpenGL** Vector Graphics library, that can run on all **OpenGL/ES** version.

Efficiently transforms a tree of shaders and samplers into a single shader at runtime.

Coming soon, Check out our website at [micro-gl.github.io/docs/nitrogl](https://micro-gl.github.io/docs/nitrogl)

## Installing `nitro{gl}`
`nitrogl` is a headers only library, which gives the following install possibilities:
1. Using `cmake` to invoke the `install` target, that will copy everything in your system via
```bash
$ mkdir cmake-build-release
$ cd cmake-build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --install .
```
2. Copying the `include/nitrogl` to anywhere you want.

## Consuming `nitro{gl}`
Following options are available:
1. copy the project to a sub folder of your project. inside your **`CMakeLists.txt`** add
```cmake
add_subdirectory(/path/to/nitrogl)
target_link_libraries(your_app nitrogl)
```
2. If you installed **`nitro{gl}`** with option 1 (see above) at your system, you can instead
```cmake
find_package(nitrogl CONFIG REQUIRED)
target_link_libraries(your_app nitrogl::nitrogl)
```
3. If you have not installed, you can add in your app's `CMakeLists.txt`
```cmake
target_include_directories(app path/to/nitrogl/folder/include/)
```
4. If you manually copied the `include/nitrogl` to the default system include path,  
you can use `cmake/Findnitrogl.cmake` to automatically create the cmake targets
```cmake
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/path/to/Findnitrogl/folder)
find_package(nitrogl REQUIRED)
target_link_libraries(your_app nitrogl::nitrogl)
```
5. Just copy the `include/nitrogl` into a sub folder of your project and include the header  
files you need with relative path in your source files.

## Running Examples
First make sure you have 
 - [SDL2](https://www.libsdl.org/) installed at your system.  
 - [cmake](https://cmake.org/download/) installed at your system.

There are two ways:
1. Use your favourite IDE to load the root `CMakeLists.txt` file, and then it   
   will pick up all of the targets, including the examples
2. Using the command line:
```bash
$ mkdir cmake-build-release
$ cd cmake-build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . --target <example_name>
$ ../examples/bin/example_name
```

```text
Author: Tomer Shalev, tomer.shalev@gmail.com, all rights reserved (2022)
```
