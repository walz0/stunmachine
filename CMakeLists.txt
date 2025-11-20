cmake_minimum_required(VERSION 3.15)
project(stunmachine)

set(CMAKE_CXX_STANDARD 17)

# === Cross-platform ENet location ===
# Windows user-provided path
if (WIN32)
    set(ENET_DIR "C:/Program Files (x86)/enet")
    include_directories("${ENET_DIR}/include")

    add_library(enet STATIC IMPORTED)
    set_target_properties(enet PROPERTIES
        IMPORTED_LOCATION "${ENET_DIR}/lib/enet.lib"
    )
else()
    # Linux: assume ENet is installed system-wide (sudo apt install libenet-dev)
    find_package(ENet REQUIRED)
endif()

# ===== CLIENT =====
add_executable(client src/client/client.cpp)

if (WIN32)
    target_link_libraries(client PRIVATE enet ws2_32 winmm)
else()
    target_link_libraries(client PRIVATE enet)   # Linux
endif()

# ===== SERVER =====
add_executable(server src/server/server.cpp)

if (WIN32)
    target_link_libraries(server PRIVATE enet ws2_32 winmm)
else()
    target_link_libraries(server PRIVATE enet)
endif()

