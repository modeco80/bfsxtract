## Bfsxtract

An extractor for "BFS" (binary file system) used in the PS2 demo "Find My Own Way".

Written in C++20.

## Building

* Clone this repository with submodules (`--recursive`)
* `cmake -Bbuild -DCMAKE_BUILD_TYPE=Release`
* `cd build`
* `make`
* ...
* Profit?

## Running

Run `bfsxtract` with FISHES.ELF next to it. 

The `out/` directory will contain extracted files.