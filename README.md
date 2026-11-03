# Vulkan Rendering Engine

[![Build and test](https://github.com/SebastiaanVryens/Vulkan-Rendering/actions/workflows/ci.yml/badge.svg)](https://github.com/SebastiaanVryens/Vulkan-Rendering/actions/workflows/ci.yml)

A 3D engine I wrote from scratch in **C++** using the **Vulkan** graphics API. It loads a 65k-triangle island map and lets you walk around it in first person. The player has gravity, can jump, and collides with the ground. A moving sun lights the scene, and it all runs at 120+ FPS.

I built it on my own in two weeks (May–June 2024) as a graphics programming course project.

![Terrain](https://github.com/SebaTheProgrammer/VulkanProject/assets/119673781/773f88ab-05c9-4557-8ba3-25b312e2e6fd)

## Highlights

- **Built directly on Vulkan.** Vulkan is a low-level graphics API. It doesn't hide anything from you, so I set up every part of the GPU work by hand: picking the graphics card, managing the images shown on screen (the swap chain), building render pipelines, allocating GPU memory, and keeping the CPU and GPU in sync.
- **First-person movement with collision.** You can walk, sprint, jump and fly. Each frame the engine casts a ray straight down from the player and tests it against every triangle in the map to find the ground.
- **Dynamic lighting.** Each pixel gets diffuse lighting from a point light plus ambient light. The light circles the scene and is drawn as a sun in the sky.
- **Data-driven scenes.** Scenes are JSON files that list which models to load, where to place them and how big they are. You can change a level without recompiling.
- **Stress-test scene.** One JSON setting fills the world with a grid of 2,000 cubes, each with a random size and rotation, to test how the renderer copes with many objects.
- **Simple physics.** A cube falls and bounces on the terrain. It uses the same gravity and collision code as the player.
- **Unit tests and CI.** GoogleTest tests check the camera math, object transforms, model loading, and the player's gravity and collision. On every push and pull request, GitHub Actions builds the engine, compiles the shaders and runs the tests.

| Moving sun lighting | Stress test with many objects |
| --- | --- |
| ![Day cycle](https://github.com/SebaTheProgrammer/VulkanProject/assets/119673781/2fafbf7d-15e6-49fa-841e-1b60c34fad31) | ![Cubes](https://github.com/SebaTheProgrammer/VulkanProject/assets/119673781/5dd30b59-c3fc-4f95-ab5a-4abea70d5d5b) |

## How it works

Every frame, the engine:

1. **Updates the game.** It reads keyboard and mouse input, moves the player, applies gravity and checks for ground collision.
2. **Sends shared data to the GPU.** The camera matrices and the sun's position go into a *uniform buffer*, a small block of GPU memory that every shader can read.
3. **Records draw commands.** Each object is drawn once. Its position, rotation and scale are sent with the draw call as *push constants*, a fast way to pass small per-object data.
4. **Submits and presents the frame.** The CPU and GPU work on two frames at once: while the GPU draws one frame, the CPU prepares the next.

More technical details:

- Meshes are loaded from `.obj` files with duplicate vertices merged. They're uploaded through staging buffers into GPU-only memory (the fastest kind) and drawn with index buffers.
- CMake compiles the GLSL shaders to SPIR-V automatically at build time.
- Resizing the window rebuilds the swap chain. Viewport and scissor are dynamic state, so the pipelines don't have to be rebuilt.
- The engine uses Mailbox presentation (low latency, no tearing) when the GPU supports it and falls back to standard V-Sync.
- Debug builds turn on Vulkan validation layers to catch mistakes in API usage.

## Tech stack

| | |
| --- | --- |
| Language | C++17 |
| Graphics API | Vulkan |
| Shaders | GLSL, compiled to SPIR-V |
| Windowing and input | GLFW |
| Math | GLM |
| Model loading | tinyobjloader |
| Scene files | nlohmann/json |
| Build system | CMake |
| Testing | GoogleTest, CTest, GitHub Actions |

## Controls

| Input | Action |
| --- | --- |
| `W` `A` `S` `D` | Move |
| Hold right mouse button and drag | Look around |
| `Space` | Jump |
| `Left Shift` | Sprint |
| `E` / `Q` | Fly up / down |
| `F` | Stop flying and fall back to the ground |

## Building and running

**You'll need:**
- The [Vulkan SDK](https://vulkan.lunarg.com/). It includes `glslc`, the shader compiler.
- CMake 3.16 or newer
- A C++17 compiler (I developed it with Visual Studio on Windows)

CMake downloads GLFW and GoogleTest automatically. It also downloads GLM if your Vulkan SDK doesn't include it.

```bash
git clone https://github.com/SebastiaanVryens/Vulkan-Rendering.git
cd Vulkan-Rendering
cmake -S . -B build
cmake --build build
```

Run `VulkanLab01` with `build/Project` as the working directory, because that's where the compiled shaders and models are copied.

**Switching scenes:** the island scene loads by default. To run the stress test, change `Models/Scene1.json` to `Models/Scene2.json` in `AppBase::LoadGameObjects()` ([Project/AppBase.cpp](Project/AppBase.cpp)).

## Running the tests

The tests are built together with the engine. Run them with CTest:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

The tests only cover code that runs on the CPU, so they don't need a GPU. The GitHub Actions build machines don't have one.

| Test file | What it checks |
| --- | --- |
| `CameraTests.cpp` | The projection matrices map the scene to Vulkan's screen and depth range. The view matrix is the exact inverse of the camera's transform. |
| `TransformTests.cpp` | Object matrices apply scale, rotation and position in the right order. Normals stay correct when an object is stretched along one axis. |
| `PhysicsTests.cpp` | The player falls with gravity, lands at the right height, jumps, and bounces. |
| `ModelLoadingTests.cpp` | Loading an `.obj` merges duplicate vertices, flips the Y axis for Vulkan, and reads the JSON model settings. |
| `UtilsTests.cpp` | The hash function used to find duplicate vertices. |

To build without the tests, configure with `-DBUILD_TESTS=OFF`.

## Project structure

| File | What it does |
| --- | --- |
| `AppBase` | Main loop: loads the scene, then updates and renders every frame |
| `EngineDevice` | Picks the GPU and creates the Vulkan device and queues |
| `SwapChain`, `Renderer` | Manage the images shown on screen, frame timing and command buffers |
| `Pipeline` | Builds graphics pipelines from compiled shaders |
| `Model`, `Buffer` | Load meshes and manage GPU memory |
| `Descriptors` | Give shaders access to the uniform buffer |
| `Systems/SimpleRenderSystem` | Draws all objects in the scene |
| `Systems/PointLightSystem` | Moves the sun and draws it |
| `Input.h` | Player controller: movement, gravity and collision |
| `SceneLoader.h` | Reads the JSON scene files |
| `Shaders/` | GLSL vertex and fragment shaders |
| `Tests/` | GoogleTest unit tests, built as a separate executable against the engine library |
| `.github/workflows/ci.yml` | The CI pipeline: builds and tests the engine on every push |

## What I'd improve next

- **GPU instancing.** Right now every cube in the stress test has its own vertex buffer and draw call. Instancing would draw them all with a single call.
- **Faster collision.** Testing the ray against every triangle works on this map, but a spatial structure like a BVH would let it scale to much bigger worlds.
- **Textures.** The texture files and `stb_image` are already in the repo but not connected yet.
- **Shadow mapping**

## Credits

- These YouTube tutorial series helped me set up the Vulkan code: [series 1](https://www.youtube.com/watch?v=W2I0DofOw9M&list=PLn3eTxaOtL2NH5nbPHMK7gE07SqhcAjmk), [series 2](https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR), [series 3](https://www.youtube.com/watch?v=dHPuU-DJoBM&list=PLv8Ddw9K0JPg1BEO-RS-0MYs423cvLVtj)
- The Wuhu Island model is from *Wii Sports Resort* © Nintendo. It's used here only for non-commercial, educational purposes.
