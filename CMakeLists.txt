cmake_minimum_required(VERSION 3.5)
project(ASCIIartconvertor)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17) #версия с++
set(CMAKE_CXX_STANDARD_REQUIRED ON) #значит обязательно использовать указанный стандарт
set(CMAKE_CXX_EXTENSIONS OFF) #значит используем стандартный с++ без расширений

add_executable(${PROJECT_NAME}_server src/server.cpp)
target_include_directories(${PROJECT_NAME}_server PRIVATE libs libs/include)

add_executable(${PROJECT_NAME}_client src/client.cpp src/FileManager.cpp src/FileManagerBK.cpp libs/sqlite3.c)
target_include_directories(${PROJECT_NAME}_client PRIVATE src libs libs/include)

add_executable(${PROJECT_NAME} src/main.cpp src/ConvertorASCII.cpp src/FileManager.cpp src/FileManagerUI.cpp src/FileManagerBK.cpp libs/sqlite3.c)
target_include_directories(${PROJECT_NAME} PRIVATE src libs libs/include)
