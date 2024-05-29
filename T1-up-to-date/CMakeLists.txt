cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(collections LANGUAGES C)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

add_executable(collections
    lib.c
    main.c
    tests.c
)

set_target_properties(
    collections PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)
