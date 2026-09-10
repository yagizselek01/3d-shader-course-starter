# Procedural Holographic Force Field



Individual project for **3D and Shader Programming**. The application renders a procedurally generated transparent sphere in OpenGL 3.3 Core and shades it as a stylized holographic force field using GLSL 330.

The main shader techniques are:

* a stylized two-sided Fresnel rim;
* animated procedural scanlines and energy bands;
* hash-based breakup;
* 2D value noise with cubic Hermite smoothing;
* noise-driven domain warping;
* two-pass rendering for the transparent convex sphere;
* distance-based shader quality LOD with hysteresis;
* GPU timing with OpenGL timer queries.

## Requirements



Primary tested course setup:

* Windows 10/11
* Visual Studio 2022 with **Desktop development with C++**
* CMake 3.24+
* Git
* a GPU/driver supporting OpenGL 3.3 Core

GLAD is included in the repository. CMake downloads pinned versions of GLFW 3.4 and GLM 1.0.3 during the first configure, so an internet connection is required at that point.

## Build



From the project root in PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

The executable is placed in:

```text
build/bin/holographic\_force\_field.exe
```

CMake also copies the shader directory to `build/bin/shaders`, so the executable can find its GLSL files when launched from the generated Visual Studio project or from the executable directory.

## Run



From the project root:

```powershell
.\\build\\bin\\holographic\_force\_field.exe
```

The console prints the OpenGL version and GPU renderer at startup.

## Controls



|Input|Action|
|-|-|
|`0`|Final hologram|
|`1`|Raw Fresnel term|
|`2`|Shaped Fresnel rim|
|`3`|Hash breakup|
|`4`|Two value-noise samples|
|`5`|Base unwarped scanline mask|
|`6`|Noise-warped scanline mask|
|`7`|Final hologram without smooth noise warp|
|`F1`|Simple quality|
|`F2`|Full quality|
|`F3`|Auto quality|
|`W` / `S`|Move camera toward / away from the sphere|
|`B`|Run GPU benchmark in Simple or Full mode|
|`Esc`|Quit|

## Quality LOD



Full quality evaluates two 2D value-noise samples per fragment and uses them to displace the coordinate domain used by the procedural effect. Simple quality skips this smooth noise warp while retaining the Fresnel rim, scanline structure, hash breakup, transparency, and animation.

Auto quality is resolved once per frame on the CPU from camera-to-sphere distance. Two different thresholds are used as hysteresis: the renderer switches to Simple beyond the far threshold and switches back to Full only after crossing the nearer threshold. This avoids rapid quality switching around one boundary.

## GPU benchmark



Press `B` while debug mode `0` is active and quality is explicitly set to `F1` or `F2`.

For each benchmark run the application:

1. resets the camera to the same position;
2. disables VSync for the duration of the benchmark;
3. keeps camera, debug mode, and quality mode fixed;
4. renders 120 warm-up frames;
5. collects 600 valid `GL\_TIME\_ELAPSED` query results around the two hologram draw calls;
6. reports average, median, 5% trimmed mean, standard deviation, minimum, and maximum GPU time;
7. restores VSync when the benchmark completes.

The benchmark measures the GPU cost of the hologram draw section under the current hardware, driver, framebuffer size, and shader build. It should not be interpreted as a percentage improvement to total application frame rate.

## Transparency



The force field uses standard alpha blending, which is order-dependent. For this convex sphere, the renderer draws back faces first and front faces second while depth testing remains enabled and depth writes are disabled. This lets both surfaces contribute to the transparent result.

This is a practical solution for the convex test object, not a general order-independent-transparency solution for arbitrary intersecting transparent geometry.

## Coordinate space



The procedural pattern is evaluated from `worldPosition.xy`. This intentionally makes the scanline/noise domain world-space based. With the current identity model matrix and centered sphere this is straightforward; moving or rotating the model would change how the world-space pattern relates to the object.

## HDR note



The fragment shader can produce RGB values above `1.0`, but the project does **not** implement a floating-point HDR render target or tone-mapping pass. Therefore this project should not be described as an HDR pipeline.

## Project structure



├── CMakeLists.txt
├── README.md
├── external/
│   └── glad/
├── shaders/
│   ├── basic.vert
│   └── basic.frag
└── src/
    ├── main.cpp
    ├── Benchmark.h / Benchmark.cpp
    ├── DebugControls.h / DebugControls.cpp
    ├── GpuTimer.h / GpuTimer.cpp
    └── QualityControls.h / QualityControls.cpp


## AI / external help

ChatGPT was used as a learning and debugging aid during development, including explanations of OpenGL/GLSL concepts, code review, refactoring suggestions, and help with benchmarking methodology. I implemented, tested, modified, and tuned the final solution myself, and I understand the submitted shader techniques and rendering decisions.

