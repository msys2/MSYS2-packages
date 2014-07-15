What we need to do:

* msys2-runtime: properly mangle paths in "complex" arguments that contain
  some arguments in one string.

* msys2-runtime: skip trying mangling arguments that contain absolute 
  windows paths with '/' as path separator. 

* ncurses: Fix it to properly work in Windows API based consoles.

* Fix key shortcuts working in bash (like Ctrl+Home, Ctrl+End)