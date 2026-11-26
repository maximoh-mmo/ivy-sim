# Ivy Sim

A deterministic procedural ivy generator implemented in native C++ as a Godot 4 GDExtension (`godot-cpp`).

> [!NOTE]
> Layer 1 (the deterministic growth simulation core, Godot API, presets, and interactive debug preview) is complete. Layer 2 (decoupled 3D tapered tube `ArrayMesh` and leaf `Transform3D` generation) is planned next.

---

## Overview

**Ivy Sim** (`ivysim`) simulates natural climbing ivy over oriented planar surfaces in C++ and exports pure topological graph data (`positions`, `thicknesses`, `parent_indices`) with zero rendering assumptions. Consumers can feed that graph into debug ribbons, 3D vine/leaf mesh builders, or gameplay systems.

### Current Features & Components

* **Deterministic RNG (`src/random/`):** Seeded **PCG32** generator (`IvyRandom`) producing identical results across runs for a given `seed`.
* **Planar Surface Geometry (`src/environment/`):** `IvyWall` supporting arbitrary 3D orientation (`origin`, orthonormal `axis_u`/`axis_v`, `width`, `height`), UV tiling/offset mapping, surface-normal computation, and tolerance-checked surface queries.
* **Surface & Attraction Sampling (`src/environment/`):** `IvyPixelSampler` and `IvySurfaceSampler` providing toroidal bilinear sampling of grayscale guidance maps (`Image.FORMAT_L8`, `FORMAT_RGB8`, `FORMAT_RGBA8` with `R == G == B` validation).
* **Hierarchical Multi-Root Graph (`src/growth/`):** `IvyPoint` and `IvyGrowthGraph` storing multi-root trees (`parent_index == -1` for roots, `parent_index < child_index` invariant) and radial thicknesses.
* **Growth Solver (`src/growth/`):** `IvyGrowthSolver` driving iterative vine propagation with:
  * Momentum, upward wall tropism (`+axis_v`), and seeded forward-hemisphere random wander
  * Damped surface-gradient ascent and iso-contour adherence (`gradient_saturation`)
  * Soft top-edge canopy band (`max_top_edge_steps`) with healthy boundary termination
  * Flat 2D spatial-grid self-avoidance steering and crowding termination
  * Probabilistic lateral branching with `branch_angle_degrees` and `min_branch_interval`
  * Pipe-model vascular thickening (`sqrt(downstream_active_tips)`), healthy boundary freeze, and dead-branch withering (`min_withered_thickness`)
  * Two-phase target waypoint reachability (Phase 1 soft steering + mandatory Phase 2 `connect_unreached_targets()` lateral tendrils)
* **Godot API & Presets (`src/ivy_generator/`, `src/settings/`):** `IvyGenerator` (`RefCounted`) and `IvySettings` (`Resource` with `.tres`/`.res` preset support and in-editor XML documentation in `doc_classes/`).
* **Interactive Godot Preview (`project/`):** `example.tscn` with a procedural brick-and-mortar attraction map, `IvyDebugRenderer`, an auto-generated `IvySettingsPanel` (live regeneration, seed stepping, `.tres` Save/Load), and click-to-place target waypoints.
* **Automated Test Suites (`tests/`, `project/tests/`):** Standalone native C++ unit tests (`bin/ivy_tests.exe` / `bin/ivy_tests`) and headless Godot integration tests (`project/tests/godot/test_runner.gd`).

---

## Quick Usage (GDScript)

```gdscript
var settings := IvySettings.new()
settings.seed = 42
settings.start_count = 3
settings.max_steps = 250
settings.branch_probability = 0.15

var generator := IvyGenerator.new()
generator.settings = settings
generator.set_wall(Vector3.ZERO, Vector3.RIGHT, Vector3.UP, 6.0, 4.0)
generator.set_attraction_image(attraction_image) # Grayscale Image (L8 / RGB8 / RGBA8)

if generator.generate():
	var positions: PackedVector3Array = generator.get_point_positions()
	var thicknesses: PackedFloat32Array = generator.get_point_thicknesses()
	var parents: PackedInt32Array = generator.get_point_parent_indices()
```

> [!TIP]
> To grow vines in from outside a visible camera frame, pass an overscanned wall to `set_wall()` (adjusting `uv_scale` and `uv_offset` to keep your attraction map aligned). To grow vines inward from side edges or downward from a ceiling, rotate `axis_u` and `axis_v` (keeping `axis_u.cross(axis_v)` equal to the wall normal).

---

## Getting Started

### Prerequisites

* **Godot 4.3+** (verified with **Godot 4.6.1**)
* **C++17 compiler** (MSVC on Windows, GCC or Clang on Linux/macOS)
* **Python 3.x** and **SCons** (`pip install scons`)

### Cloning & Building

1. Clone the repository with submodules:
   ```shell
   git clone --recursive https://github.com/maximoh-mmo/ivy-sim.git
   cd ivy-sim
   ```
   *(If already cloned without submodules, run `git submodule update --init --recursive`.)*

2. Build the GDExtension library (compiles into `project/bin/<platform>/` and embeds `doc_classes/*.xml` on `editor`/`template_debug` targets):
   ```powershell
   scons platform=windows target=template_debug -j8
   ```
   *(Or run `scons` on Linux/macOS with your platform target.)*

3. Build and run the standalone native C++ unit tests:
   ```powershell
   scons -f SConstruct.tests
   .\bin\ivy_tests.exe
   ```
   *(On Linux/macOS: `./bin/ivy_tests`.)*

4. Run the headless Godot integration tests (use the `*_console.exe` binary on Windows to capture stdout):
   ```powershell
   & "D:\Desktop\Godot_v4.6.1-stable_win64_console.exe" --headless --path project --script tests/godot/test_runner.gd
   ```

5. Open the interactive preview scene:
   Open `project/` in the Godot editor and run `example.tscn` (`Space` for next seed, Left-click on wall to add target, Right-click to remove nearest target, `C` to clear targets).

---

## Project Structure

```text
ivy-sim/
├── doc_classes/           # GDExtension XML reference docs (IvyGenerator.xml, IvySettings.xml)
├── godot-cpp/             # Godot C++ bindings submodule
├── project/               # Godot project, presets, preview scene, and integration tests
│   ├── bin/               # ivysimulation.gdextension and compiled binaries
│   ├── debug/             # ivy_debug_renderer.gd, ivy_settings_panel.gd
│   ├── presets/           # Saved IvySettings .tres presets
│   ├── tests/godot/       # test_runner.gd, sampler_tests.gd, generator_tests.gd
│   ├── example.gd         # Interactive brick-wall preview script
│   └── example.tscn       # Interactive 3D preview scene
├── src/                   # C++ extension source code
│   ├── environment/       # IvyWall, IvyPixelSampler, IvySurfaceSampler
│   ├── growth/            # IvyPoint, IvyGrowthGraph, IvyGrowthSolver
│   ├── ivy_generator/     # IvyGenerator RefCounted API
│   ├── random/            # Deterministic PCG32 IvyRandom
│   ├── registration/      # GDExtension entry point and ClassDB registration
│   └── settings/          # IvyGrowthConfig and IvySettings Resource
├── tests/                 # Native C++ and Godot bridge test code
│   ├── godot/             # IvySurfaceSamplerTestBridge
│   └── native/            # Standalone C++ unit tests
├── SConstruct             # Main GDExtension build script
└── SConstruct.tests       # Standalone native test build script
```

---

## License & Copyright

Copyright (c) 2026 Max Heinze. All rights reserved.

This project and its source code are licensed for **non-commercial use with attribution only**. Commercial use is strictly prohibited.
