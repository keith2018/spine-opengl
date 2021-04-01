cmake_minimum_required(VERSION 3.3)
project(spine-mac)

find_package(OpenGL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-exceptions -fno-rtti")

add_definitions("-Wno-unused -Wno-deprecated-declarations")

add_subdirectory(../spine-cpp spine-cpp)
add_subdirectory(../render render)

include_directories(
        ${OPENGL_INCLUDE_DIRS}
        glfw/include
        ../spine-cpp/spine-cpp/include
        ../render
)

link_directories(
        glfw/lib-x86_64/
)

FILE(GLOB MAC_SRCS "./*")
add_executable(spine-mac ${MAC_SRCS})

target_link_libraries(spine-mac
        ${OPENGL_LIBRARIES}
        "-framework Cocoa"
        "-framework IOKit"
        glfw3
        spine-render
        spine-cpp
        )
