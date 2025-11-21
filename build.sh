#!/bin/bash
set -e  # exit on any error

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

# Enter build directory
cd build

# Run CMake configuration
echo "Running CMake configure..."
cmake ..

# Build in Release mode
echo "Building..."
cmake --build . --config Release

