cmake_minimum_required(VERSION 3.10)
project(reseek)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect architecture and set appropriate flags
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    message(STATUS "Detected x86_64 architecture - using AVX2 instructions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2 -mfma")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
    message(STATUS "Detected ARM64 architecture - using NEON instructions")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+simd")
    add_definitions(-D__ARM_NEON)
else()
    message(WARNING "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR} - building without SIMD optimizations")
endif()

# Common compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -pthread -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Generate gitver.txt with current git commit hash or default value
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/gitver.txt "\"${GIT_HASH}\"\n")

# Find required packages
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# Collect all source files (excluding test files)
file(GLOB SOURCES "*.cpp" "*.c")
# Remove test files from main build
list(REMOVE_ITEM SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/test_neon.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/simple_neon_test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/raw_neon_test.cpp")

# Create executable
add_executable(reseek ${SOURCES})

# Link libraries
target_link_libraries(reseek
    ZLIB::ZLIB
    Threads::Threads
)

# Set output directory
set_target_properties(reseek PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../bin"
)

# Installation rules (optional)
install(TARGETS reseek
    RUNTIME DESTINATION bin
)