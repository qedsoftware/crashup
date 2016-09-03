Integrating your application with Crashup
=========================================

I assume that your project is CMake-based.

**NOTE:** Crashup does not require installation (it is automatically installed by Hunter when building with CMake).


1. Setup Hunter
---------------

Add Hunter Gate file to your project - you can download it from [hunter-packages/gate repository](https://github.com/hunter-packages/gate/tree/abd2dc7bac9df33a4c1aede579b2ed151c8115e2).


Setup Hunter repository (add this to the beginning of `CMakeLists.txt` before `project(...)` statement)

```
include("cmake/HunterGate.cmake")
HunterGate(
    URL "<hunter archive url>"
    SHA1 "<and its checksum>"
)
```

Where `cmake/HunterGate.cmake` is Hunter Gate file.


2. Configure CMake
------------------

Install Crashup package.

```
hunter_add_package(crashup)
find_package(crashup REQUIRED)
```


Link your executable with crashup

```
target_link_libraries(my_executable crashup::crashup)
```


And copy `crashpad_handler` executable to directory where `my_executable` is placed.

```
find_package(crashpad CONFIG REQUIRED)

add_custom_command(
    TARGET my_executable
    PRE_BUILD
    COMMAND
    "${CMAKE_COMMAND}" -E copy
    "$<TARGET_FILE:crashpad::crashpad_handler>"
    "$<TARGET_FILE_DIR:my_executable>"
)
```


3. Initialize Crashup in your application
-----------------------------------------

Create `crashup::Crashup` instance in your `main` function.
The parameters the constructor gets are:
* application name
* application version
* data directory for Crashup (i.e. place where crash minidumps will be stored)
* URL for uploading crash reports (e.g. `http://some.server:1234/submit` for Socorro if collector is running on `some.server:1234`)

Then you can set some options like throttling then.

After setting up all options call `init` method on `crashup::Crashup` instance.

4. Crashup is ready
-------------------

Crash handling is working...
