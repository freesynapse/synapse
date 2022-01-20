
#pragma once

#include "Synapse/Renderer/Material/Texture.hpp"
#include "Synapse/Core.hpp"
#include "Synapse/Memory.hpp"
#include "Synapse/Utils/Noise/Noise.hpp"

#include <functional>


namespace Syn
{

	class Texture2DNoise : public Texture
	{
	public:
		Texture2DNoise(uint32_t _side);
		~Texture2DNoise();

		void setDataDebug1D(float* _data);
		void setDataDebug2D(float* _data, const glm::vec2& _noise_limits);
		// TODO : which function in Syn::Noise to call? For now fBm of selected Syn::Noise::s_f_noise_2D_ptr
		void setFromNoiseFunc(bool _rotate_noise=true);
		void setFromNoiseData(float* _noise_data);
		void setFromPixelData(unsigned char* _px_data);

		/* Scales all pixel values with a factor _f. */
		void scale(float _f);

		/* Add another noise texture to this one, combining the two. Incoming texture is averaged with
		 * this one according to weight, where 1.0f if basically an overwrite of this texture and 0.0f
		 * is no update at all. */
		void convolute(const Ref<Texture2DNoise>& _texture_ptr=nullptr, float _weight=0.5f);

		/* Save a png image file to _filename. */
		void saveAsPNG(const std::string& _filename);

		/* Bind to texture slot. Render command.*/
		virtual void bind(uint32_t _tex_slot = 0) override;


		// accessors
		float* getNoiseData() { return m_noiseData; }


	private:
		float* m_noiseData = nullptr;
		unsigned char* m_pixelData = nullptr;


	};


}
