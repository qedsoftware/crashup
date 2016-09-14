cmake_minimum_required(VERSION 3.0.2)

option(HUNTER_ENABLED "Use Hunter package manager" YES)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.22.tar.gz"
    SHA1 "0a656ad23bf94fe95f9b9a34445a34427a476224"
)

project(crashup)

option(CRASHUP_BUILD_DEMOAPP "Crashup build demoapp" ON)

if (MSVC)
   # http://stackoverflow.com/questions/5004858/stdmin-gives-error
   # Ask politely <windows.h> not to be so stupid:
   add_definitions(-DNOMINMAX)
else ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

hunter_add_package(Qt)
find_package(Qt5Core REQUIRED) # Qt5::Core

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_USE_QTNETWORK true)

############################################################################


hunter_add_package(crashpad)
find_package(crashpad CONFIG REQUIRED)

add_library(crashup
  crashup/Crashup.cpp
)
target_link_libraries(crashup PUBLIC crashpad::crashpad_client Qt5::Core)


############################################################################

if(CRASHUP_BUILD_DEMOAPP)
  add_subdirectory(demoapp)
endif()

####
# Installation (https://github.com/forexample/package-example)

set(config_install_dir "lib/cmake/${PROJECT_NAME}")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)

# Use variables:
#   * targets_export_name
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

install(
    TARGETS crashup
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "include"
)

install(
    DIRECTORY "crashup"
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.hpp"
)

install(
    FILES "${project_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)
