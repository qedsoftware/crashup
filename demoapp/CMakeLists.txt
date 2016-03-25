cmake_minimum_required(VERSION 3.0.2)
project(demoapp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(CrashingWidget CrashingWidget.cpp)
target_link_libraries (CrashingWidget Qt5::Widgets)

add_library(Crashup ../crashup/Crashup.cpp)

add_library(Stats ../crashup/stats/Stats.cpp)

add_executable(demoapp main.cpp)
target_link_libraries (demoapp CrashingWidget)
target_link_libraries (demoapp Crashup)
target_link_libraries (demoapp Stats)
