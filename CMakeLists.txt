cmake_minimum_required(VERSION 3.18)
project(TronVanityGenerator LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Aggressive optimization flags for maximum performance
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -mavx2 -mfma -fno-signed-zeros -fno-trapping-math -funroll-loops -fvectorize")
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -mavx2 -mfma -funroll-loops")

# CUDA optimization flags
set(CMAKE_CUDA_FLAGS_RELEASE "-O3 -Xcompiler -O3 -Xcompiler -march=native -Xcompiler -mtune=native --use_fast_math")

# Additional flags for performance and security
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread -fvisibility=hidden -fstack-protector-all")

# Default to Release mode
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable CUDA
enable_language(CUDA)
find_package(CUDA 11.0 REQUIRED)

# Find required packages
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(Boost 1.70 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(libsodium REQUIRED IMPORTED_TARGET libsodium)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${libsodium_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/address_generator.cpp
    src/vanity_matcher.cpp
    src/result_writer.cpp
    src/batch_processor.cpp
    src/memory_pool.cpp
    src/performance_monitor.cpp
    src/secp256k1_utils.cpp
    src/base58_encoder.cpp
)

set(CUDA_SOURCES
    cuda/cuda_keygen.cu
    cuda/cuda_hash.cu
)

# Main executable
add_executable(tron-vanity-gen ${SOURCES} ${CUDA_SOURCES})

# Link libraries with proper ordering
target_link_libraries(tron-vanity-gen
    PRIVATE
        ${OPENSSL_LIBRARIES}
        ${CUDA_LIBRARIES}
        PkgConfig::libsodium
        pthread
        Boost::boost
        stdc++
)

# Set CUDA architecture for modern GPUs
set_target_properties(tron-vanity-gen PROPERTIES
    CUDA_ARCHITECTURES "60;70;75;80;86;90"
    CUDA_SEPARABLE_COMPILATION ON
)

# Compiler warnings
target_compile_options(tron-vanity-gen PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-unknown-pragmas>
    $<$<COMPILE_LANGUAGE:CUDA>:-Wno-unknown-pragmas>
)

# Strip binary in release for smaller footprint
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET tron-vanity-gen POST_BUILD
        COMMAND strip --strip-all $<TARGET_FILE:tron-vanity-gen>
        COMMENT "Stripping binary for optimal size"
    )
endif()

# Testing
enable_testing()

add_executable(test_keygen tests/test_keygen.cpp src/address_generator.cpp src/secp256k1_utils.cpp)
target_link_libraries(test_keygen PRIVATE ${OPENSSL_LIBRARIES} PkgConfig::libsodium)
add_test(NAME KeyGenTest COMMAND test_keygen)

add_executable(test_matcher tests/test_matcher.cpp src/vanity_matcher.cpp)
target_link_libraries(test_matcher PRIVATE pthread)
add_test(NAME MatcherTest COMMAND test_matcher)

add_executable(test_base58 tests/test_base58.cpp src/base58_encoder.cpp)
add_test(NAME Base58Test COMMAND test_base58)

# Benchmark executable
add_executable(benchmark benchmark/performance_benchmark.cpp 
    src/address_generator.cpp 
    src/performance_monitor.cpp
    src/secp256k1_utils.cpp
    src/base58_encoder.cpp
)
target_link_libraries(benchmark PRIVATE ${OPENSSL_LIBRARIES} PkgConfig::libsodium pthread)

message(STATUS "=== TRON Vanity Generator Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "CUDA Architectures: 60;70;75;80;86;90")
message(STATUS "OpenSSL: ${OPENSSL_LIBRARIES}")
message(STATUS "libsodium: Found")
message(STATUS "Optimizations: LTO, AVX2, FMA, Native March/Tune")
message(STATUS "=== Build Complete ===")
