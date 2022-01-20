
#include "pch.hpp"

#include "SynapseAddons/Fluid/Grid2D.hpp"
#include "Synapse/API/OpenGLBindings.hpp"


namespace Syn {


	//-----------------------------------------------------------------------------------
	Slab Grid2D::createSlab(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name)
	{
		std::string frontName = "";
		std::string backName = "";
		
		if (_name.compare("") != 0)
		{
			frontName = "front_" + _name;
			backName = "back_" + _name; 
		}

		Slab slab;
		slab.frontSurface = createSurface(_width, _height, _fmt, frontName);
		slab.backSurface = createSurface(_width, _height, _fmt, backName);
		return slab;
	}


	//-----------------------------------------------------------------------------------
	Ref<Framebuffer> Grid2D::createSurface(uint32_t _width, uint32_t _height, const ColorFormat& _fmt, const std::string& _name)
	{

		Ref<Framebuffer> framebuffer = API::newFramebuffer(_fmt, glm::ivec2(_width, _height), 1, true, false, _name);
		framebuffer->clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		Renderer::executeRenderCommands();

		return framebuffer;
		
	}


	//-----------------------------------------------------------------------------------
	Ref<VertexArray> Grid2D::createQuad()
	{
		
		// Screen-sized quad for rendering all the textures in their respective
		// viewports. In screen-coordinates.
		//

		glm::vec4 vertexData[] =
		{
			glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
			glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f),
			glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f),
			glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f)
		};

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		Ref<VertexBuffer> vbo = API::newVertexBuffer(GL_STATIC_DRAW);

		vbo->setBufferLayout({
			{ VERTEX_ATTRIB_LOCATION_POSITION, ShaderDataType::Float4, "a_position" }
		});
		vbo->setData(vertexData, sizeof(vertexData));

		Ref<IndexBuffer> ibo = API::newIndexBuffer(GL_TRIANGLES, GL_STATIC_DRAW);
		ibo->setData(indices, 6);

		Ref<VertexArray> vao = API::newVertexArray(vbo, ibo);

		Renderer::executeRenderCommands();

		return vao;
	}

	//-----------------------------------------------------------------------------------
	void Grid2D::clearSurface(const Ref<Framebuffer>& _frame_buffer, float _color)
	{
		_frame_buffer->clear(glm::vec4(_color));

	}


	//-----------------------------------------------------------------------------------
	void Grid2D::clearSurface(const Ref<Framebuffer>& _frame_buffer, const glm::vec4& _color)
	{
		_frame_buffer->clear(_color);

	}


	//-----------------------------------------------------------------------------------
	void Grid2D::swapSurfaces(Slab* _slab)
	{
		Ref<Framebuffer> tmp = _slab->backSurface;
		_slab->backSurface = _slab->frontSurface;
		_slab->frontSurface = tmp;
	}


}

