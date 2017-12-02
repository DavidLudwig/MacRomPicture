# MacRomPicture
Shows an .xcf bitmap/image on an emulated 68k Mac Plus (with 4 MB RAM), via C++ code, compiled into a custom Mac ROM.

This can be built using a g++ cross compiler (to m68k) from a non-m68k host.  I used a 32-bit x86 VM running Ubuntu 17.10, with the `g++-m68k-linux-gnu package` installed, via apt.  `g++` and `make` were also installed (via apt), to help out.  With those packages installed, running 'make' was sufficient to build this .rom file.

![Example Screenshot](https://user-images.githubusercontent.com/475856/33515412-01677e18-d731-11e7-95be-aea5e60dd69d.png)
