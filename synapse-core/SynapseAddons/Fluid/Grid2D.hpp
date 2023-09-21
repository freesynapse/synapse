
#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

#include "../../SynapseCore/Renderer/Mesh/MeshShape.hpp"

using namespace Syn;


namespace Syn {

	//
	typedef struct slab_ 
	{
		Ref<Framebuffer> frontSurface;
		Ref<Framebuffer> backSurface;
	} Slab;


	//
	class Grid2D
	{
	public:
		Grid2D() {};
		virtual ~Grid2D() {};

		virtual Slab createSlab(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name="");
		virtual Ref<Framebuffer> createSurface(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name);
		virtual Ref<VertexArray> createQuad();

		virtual void clearSurface(const Ref<Framebuffer>& _frame_buffer, float _color=0.0f);
		virtual void clearSurface(const Ref<Framebuffer>& _frame_buffer, const glm::vec4& _color=glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		virtual void swapSurfaces(Slab* _slab);

		//
		virtual void update(float _dt) = 0;
		virtual void render(float _dt, float _width, float _height) = 0;


	protected:
		uint32_t m_gridWidth;
		uint32_t m_gridHeight;
		float m_cellSize;
		float m_densityRadius;

		Ref<VertexArray> m_quadVAO;

	};

}

