cmake_minimum_required(VERSION 3.0)
project(aa)
set(CMAKE_C_STANDARD 99)
enable_testing()

# Tester framework
add_subdirectory(tester)

# Modules
add_subdirectory(diff)
add_subdirectory(graph)
add_subdirectory(graphio)
add_subdirectory(graphsearch)
add_subdirectory(map)
add_subdirectory(set)
add_subdirectory(queue)
add_subdirectory(var)
