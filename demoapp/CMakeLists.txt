cmake_minimum_required(VERSION 3.0.2)

project(demoapp)

set(SOCORRO_UPLOAD_URL "localhost:5000" CACHE STRING "URL where crash minidumps will be uploaded.")
configure_file(config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/crashup-demoapp/config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Qt5Widgets REQUIRED) # Qt5::Widgets

#
# MakeSegv library
#
add_library(makesegv SHARED MakeSegv.cpp)
target_link_libraries(makesegv Qt5::Core)


#
# demoapp (application for generating crashes)
#          should be called crashapp ;)
#

add_library(CrashingWidget CrashingWidget.cpp)
target_link_libraries (CrashingWidget makesegv Qt5::Widgets)

add_library(WidgetTracker WidgetTracker.cpp)
target_link_libraries (WidgetTracker Qt5::Widgets)


add_executable(demoapp main.cpp)
target_link_libraries (demoapp crashup Qt5::Widgets CrashingWidget WidgetTracker)

add_custom_command(TARGET demoapp PRE_BUILD
  COMMAND "${CMAKE_COMMAND}" -E make_directory $<TARGET_FILE_DIR:demoapp>
  COMMAND "${CMAKE_COMMAND}" -E copy
      "$<TARGET_FILE:crashpad::crashpad_handler>"
      "$<TARGET_FILE_DIR:demoapp>"
  COMMENT "Copying Crashpad Handler"
)
