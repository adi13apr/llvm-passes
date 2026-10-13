cmake_minimum_required(VERSION 3.20)
project(InstCounter LANGUAGES CXX)

# Find the installed LLVM (pass -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use LLVM's headers and compile definitions
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Prebuilt LLVM is usually compiled without RTTI; the plugin must match
if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# A MODULE library is a plugin loaded at runtime by `opt`
add_library(InstCounter MODULE InstCounter.cpp)