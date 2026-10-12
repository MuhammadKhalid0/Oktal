# Oktal

A modern C++23 library for octree-based simulation grids, built for the **Advanced Programming Techniques (AdvPT)** course (Master level, WS 2025/26) at Friedrich-Alexander-Universität Erlangen-Nürnberg (FAU).

Oktal implements a cell-based octree and uses it to run two kinds of simulations: a Poisson solver and a Lattice Boltzmann (D3Q19) fluid solver.

## Features

- **Geometry**: `Vec`, `Box` and `PeriodicBox` types.
- **Octree**: Morton-index addressing, a cell octree with iterators and cursors, octree geometry, and creation from a text descriptor.
- **Cell data**: `CellGrid` and `GridVector` for storing per-cell fields.
- **I/O**: VTK export (HTG files) for visualisation in ParaView.
- **Lattice Boltzmann**: D3Q19 kernels and a Taylor-Green vortex (TGV) setup.

## Applications

| App | Description |
| --- | --- |
| `poisson` | Solves a 3D Poisson problem on an octree grid and exports the result. |
| `tgv` | Taylor-Green vortex simulation with LBM. Usage: `tgv <refinement-level> <output-directory>` (level ≥ 5). |
| `create-htgfile` | Builds an octree from a descriptor and writes it as a VTK HTG file. Usage: `create-htgfile <filepath> <descriptor>`. |

## Project structure

```
include/oktal/   public headers (geometry, octree, data, io, lbm)
src/             library implementation
apps/            command-line applications
tests/           test suite, one folder per course task (task1 … task7)
```

## Build and test

Requirements: CMake ≥ 3.31, a C++23 compiler (CI uses clang 19).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cd build && ctest
```

The build fetches the course's helper libraries (`advpt-helpers`) at configure time. `CMakeLists.txt` also sets `CMAKE_PREFIX_PATH` to a course-cluster path for pre-installed dependencies. Outside the FAU cluster you may need to change it or install those dependencies yourself.

Warnings are treated as errors (`-Wall -Wextra -Wconversion`). Code style is enforced with `clang-format` and `clang-tidy`.

## Course context

Developed by group 07 as part of the AdvPT exercises. Each `tests/taskN` folder holds the tests for one exercise.

## Authors

Group 07, AdvPT WS 2025/26, FAU.
