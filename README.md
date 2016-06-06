Crashup
=======

Compiling demoapp
1. CMAKE_PREFIX_PATH environment variable must be set to the Qt installation
2. Run ‘cmake <source_directory>’
3. Run ‘make’
---
In case demoapp crashes immediately after being run, recompiling breakpad could solve the problem
1. Move or delete file 'google-breakpad/src/libbreakpad.a'
2. Change directory to 'google-breakpad'
3. Run './configure && make'
4. Recompile demoapp
