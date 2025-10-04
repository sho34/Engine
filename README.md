# Engine
Culpeo Game Engine

![Icon](https://media.githubusercontent.com/media/vicenteconejerosdelacruz/Engine/refs/heads/main/Engine/Target/Assets/ui/icon.png)

### Table of Content

1. Installation
2. Dependencies
    1. Assimp
    2. nlohmann::json
    3. imgui
    4. ImGuizmo
3. Architecture
    1. JSON
		1. nlohmann::json integration
		2. Macros magic
		3. JObject as base object
    2. Templates
        1. Location
        2. List of available templates
            1. Model3D
            2. Material
            3. Texture
            4. RenderPass
            5. Shader
            6. Sound
            7. *Mesh
        3. Instances
    3. Scene Objects
        1. Location
        2. List of available scene objects
            1. Renderable
            2. Camera
            3. Light
            4. SoundFX
    3. Rendering
        1. Shaders
        2. Pipeline State building
        3. Special materials
    4. Audio/Sound
        1. 2D Sounds
        2. 3D Sounds    