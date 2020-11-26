#pragma once


#include <string>

#include "src/renderer/material/Texture.h"


namespace Syn {


	class Texture2D : public Texture
	{
	public:
		Texture2D(const std::string& _asset_path);
		virtual ~Texture2D();

		virtual void bind(uint32_t _tex_slot=0) override;

		inline const std::string& getAssetPath() const { return m_assetPath; }

	private:
		std::string m_assetPath = "";
	};


}

