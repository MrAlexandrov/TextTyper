cmake_minimum_required(VERSION 3.10)

# Название проекта
project(TextTyper)

# Установить стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Определить компиляторские флаги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Добавить макрос DEBUG
add_definitions(-DDEBUG)

# Добавить исполняемый файл
add_executable(main main.cpp)

# Подключить библиотеку ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(main ${CURSES_LIBRARIES})
