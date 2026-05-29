cmake_minimum_required(VERSION 3.5)
project(ASCIIartconvertor)

set(MAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17) #версия с++
set(CMAKE_CXX_STANDARD_REQUIRED ON) #значит обязательно использовать указанный стандарт
set(CMAKE_CXX_EXTENSIONS OFF) #значит используем стандартный с++ без расширений

add_executable(${PROJECT_NAME} src/PNGtoASCII.cpp src/ConvertorASCII.cpp src/FileManager.cpp src/FileManagerUI.cpp src/FileManagerBK.cpp libs/sqlite3.c)

target_include_directories(${PROJECT_NAME} PRIVATE src libs)