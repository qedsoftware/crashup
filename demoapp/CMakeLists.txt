cmake_minimum_required(VERSION 3.0.2)
project(demoapp)

set(DESKTOP_CRASHUP_PATH ${CMAKE_CURRENT_LIST_DIR}/../)

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
   # http://stackoverflow.com/questions/5004858/stdmin-gives-error
   # Ask politely <windows.h> not to be so stupid:
   add_definitions(-DNOMINMAX)
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

#
# Setup Crashpad / Breakpad
#
if (MSVC)
    set(CRASHPAD_PATH ${DESKTOP_CRASHUP_PATH}google-crashpad/crashpad/)
    include_directories(${CRASHPAD_PATH})
    include_directories(${CRASHPAD_PATH}/third_party/mini_chromium/mini_chromium/)
elseif (UNIX)
    set(BREAKPAD_PATH ${DESKTOP_CRASHUP_PATH}/google-breakpad)
    include_directories(${BREAKPAD_PATH}/include/breakpad)
    link_directories(${BREAKPAD_PATH}/lib)
endif ()

add_library(Crashup ../crashup/Crashup.cpp)
target_link_libraries (Crashup Qt5::Core)

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

  target_link_libraries (Crashup CrashHandler CrashUploader)
endif ()

#
# Using Crashpad in Windows builds
#   TODO: use Hunter for this
#
if (MSVC)
    if (CMAKE_SIZEOF_VOID_P EQUAL 4)       # 32-bit architecture
        set(CRASHPAD_BUILD_PATH ${CRASHPAD_PATH}out/Debug)
        target_link_libraries (Crashup "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/RpcRT4.lib")
    else ()                             # 64-bit architecture
        set(CRASHPAD_BUILD_PATH ${CRASHPAD_PATH}out/Debug_x64)
        target_link_libraries (Crashup "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64/RpcRT4.lib")
    endif ()
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/handler/crashpad_handler_lib.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/minidump/crashpad_minidump.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/snapshot/crashpad_snapshot_api.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/snapshot/crashpad_snapshot.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/util/crashpad_util.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/client/crashpad_client.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/tools/crashpad_tool_support.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/compat/crashpad_compat.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/third_party/getopt/getopt.lib)
    target_link_libraries (Crashup ${CRASHPAD_BUILD_PATH}/obj/third_party/mini_chromium/mini_chromium/base/base.lib)
endif ()

############################################################################

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
