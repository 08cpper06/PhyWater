cmake_minimum_required(VERSION 3.8)

project(phy_water)

set(CMAKE_CXX_FLAGS "-O2 -std=c++17 -Wall")

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ./include)
add_subdirectory(SDL)

target_link_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/SDL)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2)


target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/SDL/include)
