cmake_minimum_required(VERSION 4.0)
project(learn)

set(CMAKE_CXX_STANDARD 17)

# Поиск Python (если переменные установлены, они будут использованы)
find_package(Python3 COMPONENTS Interpreter Development)

# Если Python найден, можно использовать его в проекте
if(Python3_FOUND)
    message(STATUS "Python найден: ${Python3_EXECUTABLE}")
    message(STATUS "Python версия: ${Python3_VERSION}")
    # Здесь можно добавить дополнительные настройки, связанные с Python
endif()
