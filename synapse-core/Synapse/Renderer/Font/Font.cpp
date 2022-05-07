
#include "pch.hpp"

#include "Synapse/Renderer/Font/Font.hpp"
#include "Synapse/Renderer/Shader/ShaderLibrary.hpp"
#include "Synapse/Renderer/Renderer.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Debug/Log.hpp"
#include "Synapse/Event/EventHandler.hpp"
#include "Synapse/Debug/Profiler.hpp"


namespace Syn { 


	Font::Font(const char* _filename, const int& _pixel_size, const Ref<Shader>& _shader)
	{
		// create and load shaders?
		if (_shader == nullptr)
		{
			SYN_CORE_TRACE("using static shaders.");

			std::string src = R"(
				#type VERTEX_SHADER
				#version 330 core

				layout(location = 0) in vec4 a_position;

				out vec2 f_tex_pos;

				void main()
				{
					gl_Position = vec4(a_position.xy, 0.0f, 1.0f);
					f_tex_pos = a_position.zw;
				}


				#type FRAGMENT_SHADER
				#version 330 core

				in vec2 f_tex_pos;

				out vec4 out_color;

				uniform sampler2D u_texture_sampler;
				uniform vec4 u_color;

				void main()
				{
					float a = texture2D(u_texture_sampler, f_tex_pos).r;
					out_color = vec4(u_color.rgb, a);
				}
			)";

			FileIOHandler::write_buffer_to_file("./static_font_shader.glsl", src);
			m_shader = MakeRef<Shader>("./static_font_shader.glsl");
			ShaderLibrary::add(m_shader);
		}
		else	// no: reuse the ones in the entered programID
		{
			m_shader = _shader;
			SYN_CORE_TRACE("reusing shader ", m_shader->getShaderID());
		}

		// allocate memory for textures
		m_texCoords.reset(new font_point[MAX_BUFFER_LENGTH * 6]);

		// allocate memory for text
		m_buffer = new char[MAX_BUFFER_LENGTH];
		memset(m_buffer, 0, MAX_BUFFER_LENGTH);
		m_tmpBuffer = new char[256];

		// reserve memory for rendering
		m_bufferOffsets.reserve(128);
		m_renderOffsets.reserve(128);

		// register resize events
		EventHandler::register_callback(EventType::VIEWPORT_RESIZE, SYN_EVENT_MEMBER_FNC(Font::onResizeEvent));

		// initialize the text atlas texture
		SYN_RENDER_S2(_filename, _pixel_size, {
			self->initAtlas(_filename, _pixel_size);
		});

		Renderer::get().executeRenderCommands();

	} // end Font::Font()


	//-----------------------------------------------------------------------------------
	Font::~Font()
	{
		SYN_RENDER_S0({
			glDeleteBuffers(1, &self->m_fontVBO);
			glDeleteVertexArrays(1, &self->m_fontVAO);
			glDeleteTextures(1, &self->m_atlasTextureID);
		});

		delete[] m_buffer;
		delete[] m_tmpBuffer;

	} // end Font::~Font()


	//-----------------------------------------------------------------------------------
	int Font::initAtlas(const char* _filename, const int& _pixel_size)
	{
		SYN_PROFILE_FUNCTION();

		// Use program for initiation of shader attributes
		auto shader = m_shader->getShaderID();
		glUseProgram(shader);

		m_uniformSampler = glGetUniformLocation(shader, "u_texture_sampler");
		m_uniformColor = glGetUniformLocation(shader, "u_color");
		
		// Init the FreeType lib
		if (FT_Init_FreeType(&m_ftLib))
		{
			Error::raise_error(nullptr, __func__, "FreeType could not be initialized.");
			return (RETURN_FAILURE);
		}
		SYN_CORE_TRACE("FreeType successfully initialized.")

		// Load the face
		SYN_CORE_TRACE("loading atlas from '", _filename, "'.");

		if (FT_New_Face(m_ftLib, _filename, 0, &m_ftFace))
		{
			SYN_CORE_ERROR("could not load atlas.");
			return (RETURN_FAILURE);
		}

		// Initialize variables before atlas creation
		FT_Set_Pixel_Sizes(m_ftFace, 0, _pixel_size);
		FT_GlyphSlot g = m_ftFace->glyph;
		m_sx = 2.0f / SCREEN_WIDTH_F;
		m_sy = 2.0f / SCREEN_HEIGHT_F;

		unsigned int roww = 0;
		unsigned int rowh = 0;

		this->m_iTextureWidth = 0;
		this->m_iTextureHeight = 0;

		memset(m_sChars, 0, sizeof(character_info_s) * MAX_CHAR_SET_SIZE);

		// Find the minimum size for a texture holding the complete ASCII charset
		for (int i = 32; i < MAX_CHAR_SET_SIZE; i++)
		{
			if (FT_Load_Char(m_ftFace, i, FT_LOAD_RENDER))
			{
				SYN_CORE_WARNING("could not log char ", i);
				continue;
			}

			if (roww + g->bitmap.width + 1 >= 1024)
			{
				m_iTextureWidth = MAX(m_iTextureWidth, roww);
				m_iTextureHeight += rowh;
				roww = 0;
				rowh = 0;
			}

			roww += g->bitmap.width + 1;
			rowh = MAX(rowh, g->bitmap.rows);
		}

		this->m_iTextureWidth = MAX(m_iTextureWidth, roww);
		this->m_iTextureHeight += rowh;

		// Setup swizzle mask for alpha texture shader rendering
		//GLint m_swizzleMask[4]; 
		//(GLint*)malloc(sizeof(GLint) * 4);
		//GLint mask[4] = { GL_ZERO, GL_ZERO, GL_ZERO, GL_RED };
		//memcpy(m_swizzleMask, mask, sizeof(GLint) * 4);

		// Create a texture to hold the character set
		glActiveTexture(GL_TEXTURE1);
		glGenTextures(1, &m_atlasTextureID);
		glBindTexture(GL_TEXTURE_2D, m_atlasTextureID);
		glUniform1i(m_uniformSampler, 1);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iTextureWidth, m_iTextureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

		// 1 byte alignment
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Clamping to edges and linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, m_swizzleMask);

		// Paste glyphs bitmaps into the texture
		//
		int ox = 0;
		int oy = 0;
		rowh = 0;

		for (int i = 32; i < MAX_CHAR_SET_SIZE; i++)
		{
			if (FT_Load_Char(m_ftFace, i, FT_LOAD_RENDER))
			{
				SYN_CORE_WARNING("could not load char bitmap ", i, " into texture.");
				continue;
			}

			if (ox + g->bitmap.width + 1 >= 1024)
			{
				oy += rowh;
				rowh = 0;
				ox = 0;
			}

			glTexSubImage2D(GL_TEXTURE_2D,
				0,
				ox,
				oy,
				g->bitmap.width,
				g->bitmap.rows,
				GL_RED,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer);

			m_sChars[i].ax = (float)(g->advance.x >> 6);
			m_sChars[i].ay = (float)(g->advance.y >> 6);
			m_sChars[i].bw = (float)g->bitmap.width;
			m_sChars[i].bh = (float)g->bitmap.rows;
			m_sChars[i].bl = (float)g->bitmap_left;
			m_sChars[i].bt = (float)g->bitmap_top;
			m_sChars[i].tx = ox / (float)m_iTextureWidth;
			m_sChars[i].ty = oy / (float)m_iTextureHeight;

			rowh = MAX(rowh, g->bitmap.rows);
			ox += g->bitmap.width + 1;

		}

		//glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, m_swizzleMask);

		// Generate VAO
		glGenVertexArrays(1, &m_fontVAO);
		glBindVertexArray(m_fontVAO);

		// Generate the VBO for fonts
		glGenBuffers(1, &m_fontVBO);

		// unbind vertex array
		glBindVertexArray(0);

		SYN_CORE_TRACE("generated ", m_iTextureWidth, "x", m_iTextureHeight, " text atlas.");

		return (RETURN_SUCCESS);


	} // end Font::InitializeAtlas()


	//-----------------------------------------------------------------------------------
	void Font::beginRenderBlock()
	{
		// clear everything
		memset(m_buffer, 0, MAX_BUFFER_LENGTH);
		m_bufferOffsets.clear();
		m_bufferOffsets.push_back(0);
		m_offset = 0;
		m_renderOffsets.clear();
		memset(m_texCoords.get(), 0, sizeof(font_point) * MAX_BUFFER_LENGTH * 6);

	}


	//-----------------------------------------------------------------------------------
	void Font::endRenderBlock()
	{
		SYN_PROFILE_FUNCTION();
		
		m_shader->enable();
		//auto self = this;
		//glUseProgram(m_shader->getShaderID());
		
		SYN_RENDER_S0({
			// Bind texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, self->m_atlasTextureID);
			glUniform1i(self->m_uniformSampler, 0);

			// Select the font VBO
			glBindVertexArray(self->m_fontVAO);
			glBindBuffer(GL_ARRAY_BUFFER, self->m_fontVBO);
			glEnableVertexAttribArray(VERTEX_ATTRIB_LOCATION_POSITION);
			glVertexAttribPointer(VERTEX_ATTRIB_LOCATION_POSITION, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);

			uint32_t c = 0;

			for (size_t i = 0; i < self->m_renderOffsets.size(); i++)
			{
				glm::vec2& pos = self->m_renderOffsets[i];

				float x = -1 + pos.x * self->m_sx;
				float y = 1 - pos.y * self->m_sy;


				// Loop through all characters

				//for (p = (const uint8_t*)self->m_buffer+offset; *p; p++) { ... }
				// OM man har det s� h�r loopar den tills '\0' vilket aldrig intr�ffar med nuvarande system
				// man f�r ist�llet ha en vanlig for-loop fr�n offset till n�sta offset och en uint8_t-cast till m_buffer[i].
				for (size_t j = self->m_bufferOffsets[i]; j < self->m_bufferOffsets[i+1]; j++)
				{
					const uint8_t* p = (const uint8_t*)(self->m_buffer + j);
					// calculate vertex and texture coordinates
					float x2 = x + self->m_sChars[*p].bl * self->m_sx;
					float y2 = -y - self->m_sChars[*p].bt * self->m_sy;
					float w = self->m_sChars[*p].bw * self->m_sx;
					float h = self->m_sChars[*p].bh * self->m_sy;

					// advance cursor
					x += self->m_sChars[*p].ax * self->m_sx;
					y += self->m_sChars[*p].ay * self->m_sy;

					// skip empty chars
					if (!w || !h)
						continue;

					self->m_texCoords[c + 0].x = x2 + w;
					self->m_texCoords[c + 0].y = -y2;
					self->m_texCoords[c + 0].s = self->m_sChars[*p].tx + self->m_sChars[*p].bw / self->m_iTextureWidth;
					self->m_texCoords[c + 0].t = self->m_sChars[*p].ty;

					self->m_texCoords[c + 1].x = x2;
					self->m_texCoords[c + 1].y = -y2;
					self->m_texCoords[c + 1].s = self->m_sChars[*p].tx;
					self->m_texCoords[c + 1].t = self->m_sChars[*p].ty;

					self->m_texCoords[c + 2].x = x2;
					self->m_texCoords[c + 2].y = -y2 - h;
					self->m_texCoords[c + 2].s = self->m_sChars[*p].tx;
					self->m_texCoords[c + 2].t = self->m_sChars[*p].ty + self->m_sChars[*p].bh / self->m_iTextureHeight;

					self->m_texCoords[c + 3].x = x2 + w;
					self->m_texCoords[c + 3].y = -y2;
					self->m_texCoords[c + 3].s = self->m_sChars[*p].tx + self->m_sChars[*p].bw / self->m_iTextureWidth;
					self->m_texCoords[c + 3].t = self->m_sChars[*p].ty;

					self->m_texCoords[c + 4].x = x2;
					self->m_texCoords[c + 4].y = -y2 - h;
					self->m_texCoords[c + 4].s = self->m_sChars[*p].tx;
					self->m_texCoords[c + 4].t = self->m_sChars[*p].ty + self->m_sChars[*p].bh / self->m_iTextureHeight;

					self->m_texCoords[c + 5].x = x2 + w;
					self->m_texCoords[c + 5].y = -y2 - h;
					self->m_texCoords[c + 5].s = self->m_sChars[*p].tx + self->m_sChars[*p].bw / self->m_iTextureWidth;
					self->m_texCoords[c + 5].t = self->m_sChars[*p].ty + self->m_sChars[*p].bh / self->m_iTextureHeight;

					c += 6;
				}
			}

			glBufferData(GL_ARRAY_BUFFER, sizeof(font_point) * c, self->m_texCoords.get(), GL_DYNAMIC_DRAW);
			glDrawArrays(GL_TRIANGLES, 0, c);

			glDisableVertexAttribArray(self->m_attributeCoord);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

		});
		//
		//glUseProgram(0);
		m_shader->disable();

	}


	//-----------------------------------------------------------------------------------
	void Font::addString(const float& _x, const float& _y, const char* _str, ...)
	{
		va_list arglist;

		if (!_str)
			return;

		// set the new buffer and store the incremented offset
		memset(m_tmpBuffer, 0, 256);
		va_start(arglist, _str);
		int offset = vsprintf(m_tmpBuffer, _str, arglist);
		va_end(arglist);

		// cpy to static buffer
		memcpy(m_buffer + m_offset, m_tmpBuffer, offset);
		m_offset += offset;

		// store render coordinates and new buffer offset
		m_renderOffsets.push_back(glm::vec2(_x, _y));
		m_bufferOffsets.push_back(m_offset);


	} // Font::RenderString_ss()


	//-----------------------------------------------------------------------------------
	void Font::setColor(const glm::vec4& _color)
	{
		m_textColor = _color;
		SYN_RENDER_S0({
			glUseProgram(self->m_shader->getShaderID());
			glUniform4fv(self->m_uniformColor, 1, (GLfloat*)(&self->m_textColor));
		});
	}

}
