How minidump_stalkwalk and dump_syms works.

Usefull links:
1.http://stackoverflow.com/questions/866721/how-to-generate-gcc-debug-symbol-outside-the-build-target
2.https://www.chromium.org/developers/decoding-crash-dumps

Files needed:
minidump_stackwalk, dump_syms.exe

On Linux.
First you need to get symbols from your application(lets name it demoapp).
You can read how to do it in  1. link

When you get the symbols in demoapp.sym file

Then you should run command:
./minidump_stackwalk yourcrash.dmp /path/to/symbols/ 2>&1 | grep demoapp.sym

You should see such output:
2016-05-13 00:17:05: simple_symbol_supplier.cc:196: INFO: No symbol file at /path/to/symbols/demoapp/6761A813078CF4CFEA744BF63D8F7D6B0/demoapp.sym

After that you should place demoapp.sym in /path/to/symbols/demoapp/6761A813078CF4CFEA744BF63D8F7D6B0/
6761A813078CF4CFEA744BF63D8F7D6B0 <- This string is connected with your build.

After that run:
./minidump_stackwalk yourcrash.dmp /path/to/symbols/ 2>&1 | grep demoapp

You wold see crash report with your symbols.




Handling windows crashes.

When you have symbols from your app in demoapp.pdb file you can run command(on windows):
dump_syms demoapp.pdb > demoapp.sym

If there was an error CoCreateInstance CLSID_DiaSource failed (msdia80.dll unregistered?)
It is connected to msdia80.dll.
After installing (old) Visual Studio on my machine error disapeared.

!!!! <IMPORTANT> !!!!!
This error keeped to appear despite registering msdia120.dll (regsrv32.exe msdia120.dll)
What solved this problem was to upgrade dump_syms.exe to the _latest_ version from the repo
!!!! </IMPORTANT> !!!!
