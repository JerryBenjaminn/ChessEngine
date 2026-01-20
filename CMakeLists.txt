cmake_minimum_required(VERSION 3.20)

project(chessengine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chessengine_core
    src/Board.cpp
    src/ConsoleRenderer.cpp
    src/MoveGen.cpp
    src/Move.cpp
    src/Search.cpp
    src/TranspositionTable.cpp
    src/OpeningBook.cpp
)

target_include_directories(chessengine_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(chessengine
    src/main.cpp
)

target_link_libraries(chessengine PRIVATE chessengine_core)

add_executable(chessengine_tests
    tests/basic_tests.cpp
)

target_link_libraries(chessengine_tests PRIVATE chessengine_core)

enable_testing()
add_test(NAME chessengine_tests COMMAND chessengine_tests)