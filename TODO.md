What we need to do:

* msys2-runtime: properly mangle paths in "complex" arguments that contain
  some arguments in one string.

* msys2-runtime: skip trying mangling arguments that contain absolute 
  windows paths with '/' as path separator. 

* ruby: Fix loading windows libraries (ticket #10).

* ncurses: Fix it to properly work in Windows API based consoles.
