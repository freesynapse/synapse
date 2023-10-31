#pragma once


#include "../Core.hpp"
#include "Renderer.hpp"
#include "./Camera/OrthographicCamera.hpp"


namespace Syn {


	class Renderer2D
	{
	public:
		//
		static void init();
		static void shutdown();

		// scene
		static void beginScene(const Syn::Ref<Syn::OrthographicCamera> &_camera);
		static void endScene();
		static void flush();

		// shaders -- for now
		static void setShader(const Ref<Shader>& _shader);

		// sprite rendering
		static void renderSprite(const glm::vec3& _pos, const glm::vec2& _size, const glm::vec4& _color);
		static void renderSprite(const glm::vec2& _pos, const glm::vec2& _size, const glm::vec4& _color);
		static void renderSprite(const glm::vec3& _pos, const glm::vec2& _size, const Ref<Texture2D>& _texture);
		static void renderSprite(const glm::vec2& _pos, const glm::vec2& _size, const Ref<Texture2D>& _texture);

		// statistics (for performance mesurements)
		struct Statistics
		{
			uint32_t drawCalls = 0;
			uint32_t spriteCount = 0;

			uint32_t getVertexCount()	{ return spriteCount * 4; }
			uint32_t getIndexCount()	{ return spriteCount * 6; }
		};

		static void resetStatistics();
		static Statistics getStatistics();

	private:
		static bool m_initalized;
	};


}



