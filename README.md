# Project Galasm

---

Read the **[User Guide](doc/user_guide.md)** to learn more!
---

## Key features:

 - Cmake is used to as build-system generator
 - Uses ccashe for debug builds (the can speed up build times significantly)
 - Build release builds
 - Gtest is the default unit-test framework
 - Debian install package
 - Switch to build with address sanitizer

## How to build compiler

```
 ./scripts/build.sh -dce

```

## How to build compiler and run unit-test

```
 ./scripts/build.sh -dceG

```

## Directory structure

```
.
├── cmake
│   ├── ProjectVersion.h.in
│   └── toolchain_clang.cmake
├── CMakeLists.txt
├── doc
│   └── user_guide.md
├── examples
│   ├── GAL22V10.chp
│   ├── GAL22V10.fus
│   ├── GAL22V10.jed
│   ├── GAL22V10.pin
│   └── GAL22V10.pld
├── galasm
│   ├── CMakeLists.txt
│   ├── inc
│   │   └── galasm
│   │       └── galasm2.h
│   ├── src
│   │   ├── galasm2.c
│   │   ├── galasm.c
│   │   ├── galasm.h
│   │   ├── galasm_types.h
│   │   ├── jedec.c
│   │   ├── jedec.h
│   │   ├── localize.c
│   │   ├── localize.h
│   │   ├── support.c
│   │   └── support.h
│   └── test
│       ├── CMakeLists.txt
│       ├── data
│       │   ├── GAL22V10.chp
│       │   ├── GAL22V10.fus
│       │   ├── GAL22V10.jed
│       │   ├── GAL22V10.pin
│       │   ├── GAL22V10.pld
│       └── test_galasm.cpp
├── galasm_main
│   ├── CMakeLists.txt
│   └── galasm_main.cpp
├── LICENSE
├── ProjectVersion.h.in
├── README.md
└── scripts
    ├── build.sh
    ├── install_benchmark.sh
    └── install_gtest.sh

```


