# Global Variable Watcher - Szymon Pawlus' submition
I implemented given specification with little extensions (like watching 1 or 2 bytes size variables)
and potential for extentability with symbols map of the process.

## Build instructions
You need to have `cmake` installed on your system.
Repo is self-contained, so you should do something along those lines:

``` sh
mkdir -p build
cd build
cmake ..
make
```

After build solution is under path `build/gwatch`
For basic running you can use fild `build/gwatch_test` that is fibbonaci program

## Running tests
Tests are build alongside the program so the only thing you need to do is run
`ctest` in build folder or go to `build/tests/tests`
It is important to remember than when you run tests manually you need to watch for working directory

## Possibilities 
- Tracking integer variable of size 1, 2, 4 or 8 bytes.
- Working for both pie and no-pie executables.
- Works for .elf format under linux.

## Known problems
- System for checking if watchpoint is read or write is very bare and inaccurate.
I though about using two watchpoints in debug registers, but it sounded really excessive and not scalable as hardware watchpoints are scarce resource (only 4 on x86)


