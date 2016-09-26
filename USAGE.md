Integrating your application with Crashup
=========================================

I assume that your project is CMake-based and uses Qt5 framework.

**NOTE:** Crashup does not require installation (it is automatically installed by Hunter when building with CMake).


1. Setup Hunter
---------------

Add Hunter Gate file to your project - you can download it from [hunter-packages/gate repository](https://github.com/hunter-packages/gate).


Setup Hunter repository (add this to the beginning of `CMakeLists.txt` before `project(...)` statement)

```cmake
include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.32.tar.gz"
    SHA1 "0bc311b87b4c1c0ab51f091e3bffb47258afb708"
)
```

Where `cmake/HunterGate.cmake` is Hunter Gate file.


2. Configure CMake
------------------

Install Crashup package.

```cmake
hunter_add_package(crashup)
find_package(crashup REQUIRED)
```


Link your executable with crashup

```cmake
target_link_libraries(my_executable crashup::crashup)
```

And copy `crashpad_handler` executable to directory where `my_executable` is placed.

```cmake
find_package(crashpad CONFIG REQUIRED)

add_custom_command(
    TARGET my_executable
    PRE_BUILD
    COMMAND
    "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:foo>"
    COMMAND
    "${CMAKE_COMMAND}" -E copy
    "$<TARGET_FILE:crashpad::crashpad_handler>"
    "$<TARGET_FILE_DIR:my_executable>"
)
```

`crash_handler` executable **must** be in the same directory as the executable that uses Crashup.


3. Initialize Crashup in your application
-----------------------------------------

I assume you create `QApplication` object in the beginning of `main` function.

```cxx
#include <QApplication>
// ...

int main(int argc, char *argv[]) {
    QApplication myapp(argc, argv);
    // ...
}
```

Create `crashup::Crashup` instance in your `main` function.

The object **must** be created after `QApplication` object, because it uses some features provided by `QApplication`.

Instead of `QApplication` you can create `QCoreApplication` for apps without UI.


The parameters the constructor gets are:
* application name
* application version
* data directory for Crashup (i.e. place where crash minidumps will be stored)
* URL for uploading crash reports (e.g. `http://some.server:1234/submit` if Socorro Collector is running on `some.server:1234`)

Then you can set some options like throttling then.

After setting up all options call `init` method on `crashup::Crashup` instance.


4. Crashup is ready
-------------------

Here are files we created:

```cmake
# CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.15.32.tar.gz"
    SHA1 "0bc311b87b4c1c0ab51f091e3bffb47258afb708"
)

project(foo)

hunter_add_package(crashup)
find_package(crashup CONFIG REQUIRED)

hunter_add_package(Qt)
find_package(Qt5 CONFIG COMPONENTS Core Widgets REQUIRED)

add_executable(foo main.cpp)
target_link_libraries(foo crashup::crashup Qt5::Core Qt5::Widgets)

find_package(crashpad CONFIG REQUIRED)

add_custom_command(
    TARGET foo
    PRE_BUILD
    COMMAND
    "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:foo>"
    COMMAND
    "${CMAKE_COMMAND}" -E copy
    "$<TARGET_FILE:crashpad::crashpad_handler>"
    "$<TARGET_FILE_DIR:foo>"
)
```

```cxx
// main.cpp

#include <crashup/Crashup.hpp>
#include <QApplication>
// ...

int main(int argc, char * argv[]) {
    QApplication myapp(argc, argv);
    // Minidumps will be stored in current working directory
    crashup::Crashup handler("App1", "v1", ".", "<your collector URL>");
    handler.init();
    // ...
}
```
