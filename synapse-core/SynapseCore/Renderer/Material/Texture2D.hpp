#pragma once


#include <string>

#include "Texture.hpp"


namespace Syn {


	class Texture2D : public Texture
	{
	public:
		/* Loads from asset, complete setup (for now). */
		Texture2D(const std::string& _asset_path);
		/* Creates an empty texture, used in conjugation with setData(). */
		Texture2D(uint32_t _width, uint32_t _height, ColorFormat _color_format=ColorFormat::RGBA8);
		virtual ~Texture2D();

		virtual void bind(uint32_t _tex_slot=0) override;

		void setData(void* _data, size_t _size);

		__always_inline const std::string& getAssetPath() const { return m_assetPath; }
		__always_inline const ColorFormat &getColorFmt() const { return m_fmt; }
		__always_inline const uint32_t getChannelCount() const { return getPixelFmtChannels(m_fmt); }
		

	private:
		std::string m_assetPath = "";

		ColorFormat m_fmt;
		OpenGLPixelFormat m_pxFmt;
	};


}

