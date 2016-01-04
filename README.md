tinyflock
=========

An interactive flocking demo

Clone and build
===============

Requires CMake. On Unix systems, simply run

    git clone --recursive https://github.com/jakogut/tinyflock
    cd tinyflock
    cmake .

to generate a Makefile (NOTE: You may first need to install other libraries, like `xorg-dev` and `libglu1-mesa-dev` on Debian-based systems)

To build on most unix platforms, run

    make

To clean all the intermediate build files, run

    make clean

To clean the project, then rebuild it, run

    make rebuild

To see what arguments the application will accept, run

    ./tinyflock --help

The demo is interactive using the mouse (left button to attract boids,
right to make them flee), as well as command line arguments, which are 
documented and viewable by running the program with the "-h" flag.

