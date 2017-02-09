# DeferredEngine

A hobby game engine written in C++ and Direct3D 10 for the purpose of exploring modern rendering and game development techniques.


- Deferred rendering and shading pipeline, with four 32-bit RTs making up the G-Buffer. Materials with diffuse, normal, specular and emissive maps",
- Template driven dynamic shader builder that eliminates unnecessary branching by generating shaders with specific feature sets
- Integrated with Newton Game Dynamics physics engine enabling car driving simulation and collisions with terrain amd dynamic voxel environment. Physics is evaluated in a multi-threaded fashion running in parallel to the rendering thread
- 3D heightmap terrain with quad-tree culling and lightmap baking
- Voxel level geometry with arbitrary tile types and automatic hidden face removal between neighboring tiles of different shapes
- Fast GPU particle system
- PCF shadow mapping
- Post-processing effects composer (bloom, gaussian blur, SSAO, FXAA)
- Custom GUI with two way data-binding and dynamic stylesheets reloaded automatically upon file changes
- Hierarchical scene-graph> with bounding volumes hierarchy
- Custom OBB collision system for voxel/terrain
- Built-in profiler with framegraph, highlighting performance chokepoints

## Dependencies

The project has the following dependencies:
- Assimp library for loading model files
- DevIL for image handling
- json-spirit as a c++ json parser
- Newton Game Dynamics
- boost 1.47

## Screenshots
			
```Voxel city  model with 100 dynamic point lights```
![Voxel city with lights](/screenshots/pic01.png?raw=true "Voxel city with lights")
  
```Shadow casting on a LeePerrySmith model```
![Face shadow](/screenshots/pic03.png?raw=true "Face shadow")
  
```Vehicle driving game with a 3d terrain```
![Vehicle](/screenshots/pic07.png?raw=true "Vehicle")
  
```Built-in editor with stylizable GUI```
![Editor](/screenshots/pic08.png?raw=true "Editor")
  
```Voxel level destruction with dynamicly generated physical objects for the debris```
![Physics](/screenshots/pic05.png?raw=true "Physics")

```Screen Space Ambient Occlusion```
![SSAO](/screenshots/pic04.png?raw=true "SSAO")
