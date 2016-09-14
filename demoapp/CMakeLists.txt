cmake_minimum_required(VERSION 3.0.2)

project(demoapp)

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

if(WIN32)
  add_custom_command(TARGET demoapp POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:crashpad::crashpad_handler>"
        "$<TARGET_FILE_DIR:demoapp>/$<TARGET_FILE_NAME:crashpad::crashpad_handler>"
    COMMENT "Copying Crashpad Handler"
  )
endif()

