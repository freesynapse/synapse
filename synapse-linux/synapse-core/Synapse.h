#pragma once


#include "pch.hpp"

#include "src/Application.h"
#include "src/Core.h"
#include "src/Types.h"
#include "src/Layer.h"
#include "src/LayerStack.h"
#include "src/SynapseMain.h"

#include "src/API/Window.h"

#include "src/debug/Log.h"
#include "src/debug/Error.h"
#include "src/debug/Profiler.h"

#include "src/event/Event.h"
#include "src/event/EventTypes.h"
#include "src/event/EventHandler.h"

#include "src/imGui/ImGuiLayer.h"

#include "src/external/imgui/imgui.h"
#include "src/external/FastNoise/FastNoise.h"

#include "src/input/InputManager.h"
#include "src/input/KeyCodes.h"

#include "src/utils/Memory.h"
#include "src/utils/FileIOHandler.h"
#include "src/utils/timer/Timer.h"
#include "src/utils/timer/TimeStep.h"
#include "src/utils/noise/Noise.h"
#include "src/utils/noise/NoiseGenerator.h"

#include "src/renderer/Renderer.h"
#include "src/renderer/RenderCommandQueue.h"
#include "src/renderer/Renderer2D.h"
#include "src/renderer/Transform.h"
#include "src/renderer/MeshCreator.h"

#include "src/renderer/buffers/VertexArray.h"
#include "src/renderer/buffers/VertexBuffer.h"
#include "src/renderer/buffers/IndexBuffer.h"
#include "src/renderer/buffers/Framebuffer.h"

#include "src/renderer/mesh/MeshDebug.h"
#include "src/renderer/mesh/MeshAssimp.h"
#include "src/renderer/mesh/MeshShape.h"

#include "src/renderer/camera/OrthographicCamera.h"
#include "src/renderer/camera/PerspectiveCamera.h"

#include "src/renderer/font/Font.h"

#include "src/renderer/shader/Shader.h"
#include "src/renderer/shader/ShaderLibrary.h"

#include "src/renderer/material/Texture2D.h"


