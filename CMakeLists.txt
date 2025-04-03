cmake_minimum_required(VERSION 3.30)
project(parvpo5)

find_package(SQLite3 REQUIRED)
set(CMAKE_CXX_STANDARD 20)

add_executable(parvpo5 main.cpp DatabaseDiagramGenerator/DatabaseDiagramGenerator.cpp
        DatabaseDiagramGenerator/DatabaseDiagramGenerator.h)

target_link_libraries(parvpo5 PRIVATE SQLite::SQLite3)