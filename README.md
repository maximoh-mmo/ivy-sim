# Ivy Sim

A procedural ivy generation C++ GDExtension for Godot 4.

> [!NOTE]
> This project is an active **work in progress**. It is being developed as a high-performance procedural ivy generation tool for use in an upcoming game project.

---

## Overview

**Ivy Sim** (`ivysim`) provides C++ accelerated algorithms for simulating natural plant and ivy growth over 3D geometry in Godot. It is designed to simulate realistic climbing, branching, adhesion to surfaces, and surface attraction mapping.

### Current Features & Components

* **Deterministic RNG (`src/random/`):** Seeded, reproducible pseudo-random number generator powered by the **PCG32** algorithm, supporting integer ranges, uniform booleans, and floating-point distributions.
* **Planar Surface Geometry (`src/environment/`):** `IvyWall` representation supporting world-to-local and local-to-world coordinate transformations, dynamic normal calculations, bounding checks, and tolerance-based surface testing.
* **Surface & Attraction Sampling (`src/environment/`):** `IvyPixelSampler` and `IvySurfaceSampler` supporting bilinear interpolation, UV coordinate wrapping, and Godot `Image` format loading (L8, RGB8, RGBA8) for attraction guidance maps.
* **Modular Simulation Architecture:** Domain-focused architecture structured into `environment/`, `growth/`, `random/`, `registration/`, and `settings/`.
* **Standalone Unit Test Suite (`tests/`):** Cross-platform standalone test runner verifying math, RNG sequences, coordinate transformations, and sampling behavior.

---

## Getting Started

### Prerequisites

* **Godot 4.x** (targeting 4.3+ GDExtension baseline)
* **C++17 compiler** (MSVC on Windows, GCC or Clang on Linux/macOS)
* **Python 3.x** and **SCons** (`pip install scons`)

### Cloning & Building

1. Clone the repository with submodules:
   ```shell
   git clone --recursive https://github.com/maximoh-mmo/ivy-sim.git
   cd ivy-sim
   ```
   *(If already cloned without submodules, run `git submodule update --init --recursive`)*

2. Build the GDExtension library:
   ```shell
   scons
   ```
   This compiles the extension and places the compiled library into `project/bin/<platform>/`.

3. Run the standalone unit tests:
   ```shell
   scons -f SConstruct.tests
   ./bin/ivy_tests
   ```

4. Open the Godot test project:
   Launch the Godot editor, import the `project/` folder, and run the main scene to test GDExtension initialization.

---

## Project Structure

```text
ivy-sim/
├── doc_classes/       # GDExtension XML documentation
├── godot-cpp/         # Godot C++ bindings submodule
├── project/           # Godot project to test and run the extension
│   ├── bin/           # GDExtension config and compiled binaries
│   └── example.tscn   # Test scene
├── src/               # C++ extension source code
│   ├── environment/   # Surface geometry, walls, and attraction samplers
│   ├── growth/        # Ivy growth solvers and simulation points
│   ├── ivy_generator/ # Core IvyGenerator class
│   ├── random/        # Deterministic PCG32 random number generator
│   ├── registration/  # GDExtension initialization and type registration
│   └── settings/      # Simulation parameters and configuration
├── tests/             # Standalone C++ unit tests
├── SConstruct         # Main extension build script
└── SConstruct.tests   # Test suite build script
```

---

## License & Copyright

Copyright (c) 2026 Max Heinze. All rights reserved.

This project and its source code are licensed for **non-commercial use with attribution only**. Commercial use is strictly prohibited.
