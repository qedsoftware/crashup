Crashup
=======

Compiling demoapp
-----------------
1. (Linux only) Get and build Google Breakpad: `./scripts/build-breakpad.sh`
2. Check if `CMAKE_PREFIX_PATH` environment variable is set to the Qt installation
3. Create directory for build files and run `cmake <source directory>`
4. Run `make`

Notes
-----

When writing your own app do not forget to copy `crashpad_handler` (Windows).
For example see `demoapp/CMakeLists.txt`.
