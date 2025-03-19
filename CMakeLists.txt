cmake_minimum_required(VERSION 3.10)
project(SimpleRaylibGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Add raylib as a subdirectory
add_subdirectory(raylib)

# Add source files
set(SOURCES
    main.c
    circle.c
    camera.c
    debug.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link raylib
target_link_libraries(${PROJECT_NAME} raylib) 