cmake_minimum_required(VERSION 3.16)
project(jts2gd VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(jts2gd src/main.cpp src/lexer.cpp src/js_parser.cpp src/cgen.cpp)

if (WIN32)
    target_compile_options(jts2gd PRIVATE /W3)
    set(CMAKE_CXX_FLAGS_DEBUG "/Z7" CACHE STRING "Flags used by the CXX compiler during DEBUG builds" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2" CACHE STRING "Flags used by the CXX compiler during RELEASE builds" FORCE)
    set(CMAKE_CXX_FLAGS_ASAN "/Z7 /fsanitize=address" CACHE STRING "Flags used by the CXX compiler during ASAN builds" FORCE)

elseif (UNIX)
    target_compile_options(jts2gd PRIVATE -Wall -Wextra)
    set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "Flags used by the CXX compiler during DEBUG builds" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native" CACHE STRING "Flags used by the CXX compiler during RELEASE builds" FORCE)
    set(CMAKE_CXX_FLAGS_ASAN "-g -fsanitize=address" CACHE STRING "Flags used by the CXX compiler during ASAN builds" FORCE)

endif()





set(allowableBuildTypes Debug Release Asan)
if (NOT CMAKE_BUILD_TYPE IN_LIST allowableBuildTypes)
    message(FATAL_ERROR "${CMAKE_BUILD_TYPE} is not a defined build type")
endif()
