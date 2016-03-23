cmake_minimum_required(VERSION 3.0.2)
project(demoapp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Widgets)
find_package (Threads REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(CrashingWidget CrashingWidget.cpp)
target_link_libraries (CrashingWidget Qt5::Widgets)

############################################################################
### path to desktop_crashup project ########################################
############################################################################
set(DESKTOP_CRASHUP_PATH ${CMAKE_CURRENT_LIST_DIR}/../)

############################################################################
### set path to the /src/ directory in your google-breakpad installation ###
### + include this directory ###############################################
############################################################################
set(BREAKPAD_SRC_PATH ${DESKTOP_CRASHUP_PATH}/google-breakpad/src/)
include_directories(${BREAKPAD_SRC_PATH})

add_library(Crashup ../crashup/Crashup.cpp)
target_link_libraries (Crashup Qt5::Core)

add_library(Stats ../crashup/stats/Stats.cpp)

add_library(CrashHandler ../crashup/crash_handler/CrashHandler.cpp)
############################################################################
### breakpad static client library #########################################
############################################################################
target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/linux/libbreakpad_client.a)
target_link_libraries (CrashHandler Qt5::Core)
target_link_libraries (CrashHandler ${CMAKE_THREAD_LIBS_INIT})

add_executable(demoapp main.cpp)
target_link_libraries (demoapp CrashingWidget)
target_link_libraries (demoapp Crashup)
target_link_libraries (demoapp Stats)
target_link_libraries (demoapp CrashHandler)
