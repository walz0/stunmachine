cmake_minimum_required(VERSION 3.15)
project(stunmachine)

#set(CMAKE_C_COMPILER "C:/ProgramData/mingw64/mingw64/bin/gcc.exe")
#set(CMAKE_CXX_COMPILER "C:/ProgramData/mingw64/mingw64/bin/g++.exe")
set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
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
	# Linux: system ENet installation
    include_directories("/usr/include")
    # The actual lib file
    set(ENET_LIB "/usr/lib/x86_64-linux-gnu/libenet.so")
endif()

# ===== CLIENT =====
if (WIN32)
	add_executable(client src/client.cpp)
    target_link_libraries(client PRIVATE enet ws2_32 winmm)
else()
endif()

# ===== SERVER =====
add_executable(server src/server.cpp)

if (WIN32)
    target_link_libraries(server PRIVATE enet ws2_32 winmm)
else()
    target_link_libraries(server PRIVATE enet)
endif()

