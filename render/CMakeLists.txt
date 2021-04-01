file(GLOB SOURCES "utils/*.h" "*.cpp")

include_directories(
        ../spine-cpp/spine-cpp/include
)

add_library(spine-render STATIC ${SOURCES})
