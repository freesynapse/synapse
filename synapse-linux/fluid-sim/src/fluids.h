
#pragma once

#include "glad/glad.h"


typedef struct surface_descriptor_
{
	uint32_t width;
	uint32_t height;
	uint32_t numComponents;
	bool useHalfFloats;
	
} SurfaceDesc;

typedef struct surface_
{
	GLuint fboID;
	GLuint textureID;
	SurfaceDesc desc;

} Surface;

typedef struct slab_
{
	Surface frontSurface;
	Surface backSurface;

} Slab;



class Fluid
{
public:
	static Slab createSlab(uint32_t _width, uint32_t _height, uint32_t _num_compononents);
	static Surface createSurface(uint32_t _width, uint32_t _height, uint32_t _num_compononents, bool _use_half_floats=true);




};
