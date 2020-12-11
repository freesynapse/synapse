#pragma once


#include "pch.hpp"

#include "Synapse/Application.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Types.hpp"
#include "Synapse/Layer.hpp"
#include "Synapse/LayerStack.hpp"
//#include "Synapse/SynapseMain.hpp"

#include "Synapse/API/Window.hpp"
#include "Synapse/API/SynapseOpenGLBindings.hpp"

#include "Synapse/Debug/Log.hpp"
#include "Synapse/Debug/Error.hpp"
#include "Synapse/Debug/Profiler.hpp"

#include "Synapse/Event/Event.hpp"
#include "Synapse/Event/EventTypes.hpp"
#include "Synapse/Event/EventHandler.hpp"

#include "Synapse/ImGui/ImGuiLayer.hpp"

#include "Synapse/External/imgui/imgui.h"
#include "Synapse/External/FastNoise/FastNoise.h"

#include "Synapse/Input/InputManager.hpp"
#include "Synapse/Input/KeyCodes.hpp"

#include "Synapse/Utils/Memory.hpp"
#include "Synapse/Utils/FileIOHandler.hpp"
#include "Synapse/Utils/Timer/Timer.hpp"
#include "Synapse/Utils/Timer/TimeStep.hpp"
#include "Synapse/Utils/Noise/Noise.hpp"
#include "Synapse/Utils/Noise/NoiseGenerator.hpp"

#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Renderer/RenderCommandQueue.hpp"
#include "Synapse/Renderer/Renderer2D.hpp"
#include "Synapse/Renderer/Transform.hpp"
#include "Synapse/Renderer/MeshCreator.hpp"

#include "Synapse/Renderer/Buffers/VertexArray.hpp"
#include "Synapse/Renderer/Buffers/VertexBuffer.hpp"
#include "Synapse/Renderer/Buffers/IndexBuffer.hpp"
#include "Synapse/Renderer/Buffers/Framebuffer.hpp"

#include "Synapse/Renderer/Mesh/MeshDebug.hpp"
#include "Synapse/Renderer/Mesh/MeshAssimp.hpp"
#include "Synapse/Renderer/Mesh/MeshShape.hpp"

#include "Synapse/Renderer/Camera/OrthographicCamera.hpp"
#include "Synapse/Renderer/Camera/PerspectiveCamera.hpp"

#include "Synapse/Renderer/Font/Font.hpp"

#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"

#include "Synapse/Renderer/Material/Texture2D.hpp"

