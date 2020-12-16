cmake_minimum_required(VERSION 3.10)
project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(cpprestsdk REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE cpprestsdk::cpprest wiringPi boost_system)
