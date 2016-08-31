cmake_minimum_required(VERSION 3.0.2)

if(WIN32)
  option(HUNTER_ENABLED "Use Hunter package manager" YES)
else()
  option(HUNTER_ENABLED "Use Hunter package manager" NO)
endif()

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.22.tar.gz"
    SHA1 "0a656ad23bf94fe95f9b9a34445a34427a476224"
)

project(crashup)

if (MSVC)
   # http://stackoverflow.com/questions/5004858/stdmin-gives-error
   # Ask politely <windows.h> not to be so stupid:
   add_definitions(-DNOMINMAX)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

find_package(Qt5 COMPONENTS Core Widgets REQUIRED) # Qt5::Widgets, Qt5::Core

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTNETWORK true)

############################################################################

#
# Setup Crashpad / Breakpad
#
if(UNIX)
    set(BREAKPAD_PATH ${CMAKE_CURRENT_LIST_DIR}/google-breakpad)
    include_directories(${BREAKPAD_PATH}/include/breakpad)
    link_directories(${BREAKPAD_PATH}/lib)
elseif (WIN32)
    hunter_add_package(crashpad)
    find_package(crashpad CONFIG REQUIRED)
endif()

#
# Crash handling and uploading for Linux
#
if (UNIX)
  find_package(Threads REQUIRED)
  add_library(crashup
    crashup/Crashup.cpp

    crashup/crash_handler/CrashHandler.cpp
    crashup/crash_handler/CrashUploader.cpp

    crashup/crash_handler/libcurl_wrapper.cc
    crashup/crash_handler/google_crashdump_uploader.cc
  )
  target_link_libraries(crashup PUBLIC Qt5::Core libbreakpad_client.a ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

elseif (WIN32)
  add_library(crashup
    crashup/Crashup.cpp
  )
  target_link_libraries(crashup PUBLIC crashpad::crashpad_client Qt5::Core)

endif()

############################################################################

add_subdirectory(demoapp)
