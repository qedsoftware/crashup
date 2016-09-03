Crashup
=======

Compiling demoapp
-----------------

Create directory for build files and generate build tree:

```
cd <source directory>
mkdir _build
cd _build
cmake .. -DSOCORRO_UPLOAD_URL="<Socorro minidump upload URL>"
```

Run `make` or `cmake --build .` (esp. on Windows second option is recommended)


Usage
-----

For instructions how to integrate your app with Crashup, see [USAGE.md](USAGE.md).

