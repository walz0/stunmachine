@echo off
setlocal

if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

echo Running CMake configure...
cmake ..

echo Building...
cmake --build . --config Release

endlocal

