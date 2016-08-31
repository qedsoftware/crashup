cmake_minimum_required(VERSION 3.0.2)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.22.tar.gz"
    SHA1 "0a656ad23bf94fe95f9b9a34445a34427a476224"
)

project(demoapp)

if (MSVC)
   # http://stackoverflow.com/questions/5004858/stdmin-gives-error
   # Ask politely <windows.h> not to be so stupid:
   add_definitions(-DNOMINMAX)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

find_package(Qt5Widgets REQUIRED) # Qt5::Widgets, Qt5::Core

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTNETWORK true)

add_library(CrashingWidget CrashingWidget.cpp)
target_link_libraries (CrashingWidget Qt5::Widgets)

add_library(WidgetTracker WidgetTracker.cpp)
target_link_libraries (WidgetTracker Qt5::Widgets)

############################################################################

#
# Setup Crashpad / Breakpad
#
if(UNIX)
    set(BREAKPAD_PATH ${DESKTOP_CRASHUP_PATH}/google-breakpad)
    include_directories(${BREAKPAD_PATH}/include/breakpad)
    link_directories(${BREAKPAD_PATH}/lib)
elseif (WIN32)
    hunter_add_package(crashpad)
    find_package(crashpad CONFIG REQUIRED)
endif()

add_library(Crashup ../crashup/Crashup.cpp)

#
# Crash handling and uploading for Linux
#
if (UNIX)
  # Uploading...
  add_library(LibcurlWrapper ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/libcurl_wrapper)
  target_link_libraries(LibcurlWrapper ${CMAKE_DL_LIBS})

  add_library(GoogleCrashUploader ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/google_crashdump_uploader)
  target_link_libraries(GoogleCrashUploader LibcurlWrapper)

  add_library(CrashUploader ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/CrashUploader.cpp)
  target_link_libraries (CrashUploader GoogleCrashUploader Qt5::Core)

  # Handling crashes...
  add_library(CrashHandler ../crashup/crash_handler/CrashHandler.cpp)
  target_link_libraries (CrashHandler libbreakpad_client.a Qt5::Core ${CMAKE_THREAD_LIBS_INIT})

  target_link_libraries (Crashup Qt5::Core CrashHandler CrashUploader)

elseif (WIN32)
  target_link_libraries(Crashup PUBLIC crashpad::crashpad_client Qt5::Core)
  add_custom_command(TARGET Crashup POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:crashpad::crashpad_handler>"
        "$<TARGET_FILE_DIR:Crashup>/$<TARGET_FILE_NAME:crashpad::crashpad_handler>"
    COMMENT "Copying Crashpad Handler"
  )

endif()
#
# MakeSegv library
#
add_library(MakeSegv SHARED MakeSegv.cpp)
target_link_libraries(MakeSegv Qt5::Core)

#
# demoapp (application for generating crashes)
#          should be called crashapp ;)
#
add_executable(demoapp main.cpp)
target_link_libraries (demoapp CrashingWidget)
target_link_libraries (demoapp WidgetTracker)
target_link_libraries (demoapp Crashup)
target_link_libraries (demoapp MakeSegv)
