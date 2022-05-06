#pragma once
#pragma warning(disable : 4005)


#include <ft2build.h>
#include <freetype/freetype.h>

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Event/EventTypes.hpp"


namespace Syn {	

	struct font_point_s
	{
		GLfloat x, y, s, t;
		font_point_s(float _x, float _y, float _s, float _t) : x(_x), y(_y), s(_s), t(_t) {}
		font_point_s() : x(0.0f), y(0.0f), s(0.0f), t(1.0f) {}
	};
	typedef font_point_s font_point;


	struct character_info_s
	{
		float ax, ay;
		float bw, bh;
		float bl, bt;
		float tx, ty;
	};


	class Font
	{
	public:
		// Constructor / destructor
		Font(const char *_filename, const int& _pixel_size=12, const Ref<Shader>& _shader=nullptr);
		~Font();

		void beginRenderBlock();
		void endRenderBlock();
		void addString(const float& _x, const float& _y, const char* _str, ...);
		

		// Accessors
		inline const void enableShader() const { m_shader->enable(); }
		inline const void disableShader() const { m_shader->disable(); }
		inline const unsigned int& getFontHeight() const { return (m_iTextureHeight); }
		inline Ref<Shader> getShader() const { return (m_shader); }
		inline const glm::vec4& getColor() const { return (m_textColor); }
		void setColor(const glm::vec4& _color);
		
		void onResizeEvent(Event* _e)
		{
			ViewportResizeEvent* e = dynamic_cast<ViewportResizeEvent*>(_e);
			m_sx = 2.0f / static_cast<float>(e->getViewportX());
			m_sy = 2.0f / static_cast<float>(e->getViewportY());
		}
		//void setScreenRes(float _width, float _height)
		//{
		//	m_sx = 2.0f / _width;
		//	m_sy = 2.0f / _height;
		//}


	private:
		int initAtlas(const char* _filename, const int& _pixel_size);


	private:
		// .FreeType
		FT_Library m_ftLib;
		FT_Face m_ftFace;
		FT_GlyphSlot m_glyph;

		// .texture
		GLuint m_fontTextureID = 0;
		GLuint m_atlasTextureID = 0;
		unsigned int m_iTextureWidth = 0;
		unsigned int m_iTextureHeight = 0;
		//GLint *m_swizzleMask;

		// .atlas
		char* m_buffer;
		char* m_tmpBuffer;
		std::vector<uint32_t> m_bufferOffsets;
		uint32_t m_offset;
		std::vector<glm::vec2> m_renderOffsets;
		static constexpr int MAX_BUFFER_LENGTH = 8192;
		static constexpr int MAX_CHAR_SET_SIZE = 128;
		character_info_s m_sChars[MAX_CHAR_SET_SIZE];
		std::unique_ptr<font_point[]> m_texCoords;

		// .vbo
		GLuint m_fontVAO = 0;
		GLuint m_fontVBO = 0;

		// .GLSL shaders
		Ref<Shader> m_shader = nullptr;
		
		// .shader parameter locations
		GLuint m_uniformSampler = 0;
		GLuint m_uniformColor = 0;
		GLuint m_attributeCoord = 0;
		glm::vec4 m_textColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

		// .text rendering attributes
		float m_sx = 0.0f;
		float m_sy = 0.0f;

	};

}

