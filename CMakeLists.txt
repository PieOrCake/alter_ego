cmake_minimum_required(VERSION 3.20)

# Cross-compilation setup for Windows on Linux (must be before project())
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

project(AlterEgo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build for optimized, stripped output
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG")

# Static linking to avoid Windows DLL dependencies
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -DWIN32_LEAN_AND_MEAN")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui)
include_directories(${CMAKE_SOURCE_DIR}/lib/imgui/backends)
include_directories(${CMAKE_SOURCE_DIR}/lib/nlohmann)

# ImGui source files
set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
)

# Source files
set(SOURCES
    src/dllmain.cpp
    src/Clears.cpp
    src/GW2API.cpp
    src/IconManager.cpp
    src/ChatLink.cpp
    src/HttpClient.cpp
    src/SkinCache.cpp
    src/OwnedSkins.cpp
    src/Commerce.cpp
    src/WikiImage.cpp
    ${IMGUI_SOURCES}
)

# Create shared library (DLL)
add_library(AlterEgo SHARED ${SOURCES})

# Force Windows DLL extension
set_target_properties(AlterEgo PROPERTIES
    SUFFIX ".dll"
    PREFIX ""
)

# Link Windows libraries for Win32 and networking
target_link_libraries(AlterEgo
    gdi32
    wininet
    shell32
)

# Module definition file for clean DLL export
target_link_options(AlterEgo PRIVATE -Wl,--out-implib,AlterEgo.lib -Wl,--strip-all)
target_sources(AlterEgo PRIVATE ${CMAKE_SOURCE_DIR}/AlterEgo.def)

# Output directory
set_target_properties(AlterEgo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
