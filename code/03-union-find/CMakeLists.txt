# Build and run:
#   cmake -S . -B build
#   cmake --build build --config Release   # --config is required on Windows/Xcode
#   ./build/union_find                     # the table in the post (build/Release/ on Windows)
#   ./build/find_bench                     # the read-heavy comparison
cmake_minimum_required(VERSION 3.16)
project(union_find CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# never benchmark a debug build. Multi-config generators (Visual Studio, Xcode)
# ignore CMAKE_BUILD_TYPE and pick the config at build time, so pin them to Release.
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
  set(CMAKE_CONFIGURATION_TYPES Release CACHE STRING "" FORCE)
elseif(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_executable(union_find union_find.cpp)
add_executable(find_bench find_bench.cpp)
