cmake_minimum_required(VERSION 3.0.2)
project(demoapp)
if (MSVC)
  # force linking UCRT statically ("/MTd" or "/MT" flag in "CL.exe")
  # because google breakpad compilation links them statically and we don't
  # want to touch it, so we change here to static instead of dynamic linking
  foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
   if(${flag_var} MATCHES "/MDd")
      string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MDd")
   endforeach(flag_var)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

find_package (Qt5Widgets)
find_package (Threads REQUIRED)
find_package (Qt5Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTNETWORK true)

add_library(CrashingWidget CrashingWidget.cpp)
target_link_libraries (CrashingWidget Qt5::Widgets)

add_library(WidgetTracker WidgetTracker.cpp)
target_link_libraries (WidgetTracker Qt5::Widgets)

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
add_library(Downloader ../crashup/updates/Downloader.cpp)
target_link_libraries (Downloader Qt5::Core)
target_link_libraries (Downloader Qt5::Network)

#################################
### uploading crashes linking ###
#################################
if (UNIX)
  add_library(LibcurlWrapper ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/libcurl_wrapper)
  target_link_libraries(LibcurlWrapper ${CMAKE_DL_LIBS})
  add_library(GoogleCrashUploader ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/google_crashdump_uploader)
  target_link_libraries(GoogleCrashUploader LibcurlWrapper)
endif (UNIX)

add_library(CrashUploader ${DESKTOP_CRASHUP_PATH}/crashup/crash_handler/CrashUploader.cpp)
if (UNIX)
  target_link_libraries (CrashUploader GoogleCrashUploader)
endif (UNIX)
target_link_libraries (CrashUploader Qt5::Core)

add_library(CrashHandler ../crashup/crash_handler/CrashHandler.cpp)
############################################################################
### breakpad static client library #########################################
############################################################################
if (MSVC)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/common.lib)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/crash_generation_client.lib)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/crash_generation_server.lib)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/crash_report_sender.lib)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/exception_handler.lib)
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/windows/Debug/lib/processor_bits.lib)
else ()
    target_link_libraries (CrashHandler ${BREAKPAD_SRC_PATH}/client/linux/libbreakpad_client.a)
endif ()
target_link_libraries (CrashHandler Qt5::Core)
target_link_libraries (CrashHandler ${CMAKE_THREAD_LIBS_INIT})

add_executable(demoapp main.cpp)
target_link_libraries (demoapp CrashingWidget)
target_link_libraries (demoapp WidgetTracker)
target_link_libraries (demoapp Crashup)
target_link_libraries (demoapp Stats)
target_link_libraries (demoapp Downloader)
target_link_libraries (demoapp CrashHandler)
target_link_libraries (demoapp CrashUploader)
