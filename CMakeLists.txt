cmake_minimum_required(VERSION 3.15)
project(stunmachine)

set(CMAKE_CXX_STANDARD 17)

# Path to your ENet folder
set(ENET_DIR "C:/Program Files (x86)/enet")

# Add include path
include_directories("${ENET_DIR}/include")

# Import ENet library
add_library(enet STATIC IMPORTED)
set_target_properties(enet PROPERTIES
    IMPORTED_LOCATION "${ENET_DIR}/lib/enet.lib"
)

# Add your source files from the src folder
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_executable(stunmachine ${SOURCES})

# Link ENet + required Windows libs
target_link_libraries(stunmachine PRIVATE
    enet
    ws2_32
    winmm
)

