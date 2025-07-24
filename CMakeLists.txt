cmake_minimum_required(VERSION 3.15)
project(toor_craft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the Field library as a subdirectory
add_subdirectory(src)

add_subdirectory(examples)