# Molesim - a simulator and visualizer for molecules.

## Build dependencies
On non-Windows platform SDL2 must be installed. On debian based distributions it can be installed with the following command:
```
sudo apt install libsdl2-2.0-0
```

## Build from source

To clone the project and its dependencies:

```
git clone --recurse-submodules https://github.com/ronsaldo/molesim
```


The project is using CMake, you may first create a build folder with the following command at the root of the project:

```
mkdir build
cd build
```

On Windows, you can configure and build the project with your favorite IDE or with:

```
cmake ..
cmake --build . --config RelWithDebInfo
```

On other platforms, you may need the following commands to get a RelWithDebInfo build:

```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

After building, executable and resource folders are located in the `build/dist` folder.
