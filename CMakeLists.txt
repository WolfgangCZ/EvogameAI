cmake_minimum_required(VERSION 3.20)
project(evo_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Fetch Raylib ──────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)

# Disable Raylib extras we don't need
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)

# ── Fetch raygui ──────────────────────────────────────────────────
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raygui)

# ── Sources ───────────────────────────────────────────────────────
file(GLOB SOURCES src/*.c)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include ${raygui_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# ── Platform link extras ──────────────────────────────────────────
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 gdi32 winmm)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL" "-framework Cocoa" "-framework IOKit")
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL m pthread dl rt X11)
endif()

# ── Compiler warnings ─────────────────────────────────────────────
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>: -Wall -Wextra>
    $<$<C_COMPILER_ID:MSVC>:      /W4>
)
