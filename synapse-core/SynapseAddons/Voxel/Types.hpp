
#pragma once

#include <glm/glm.hpp>


//---------------------------------------------------------------------------------------
// Voxel related macros and inlines
//

/* Basic debugging of the chunk manager class(es) */
//#define DEBUG_VX_CHUNK_MANAGER

/* Auto spawn/despawn chunk based on camera positions? */
//#define VX_AUTO_LOAD_CHUNKS

/* Use threaded meshing? -- performance increase */
#define VX_THREADED_MESHING

/* Use even greedier meshing! - skip rendering unseen between-chunk boundaries
 * N.B.: if frequent chunk multiple updates are needed this may be a performance hit... */
#define VX_CHUNK_STITCHING

// extract bits from the Voxel.data bitfield
#define VX_DATA_MASK_LOW8			0x000000ff
#define VX_DATA_MASK_TYPE			0x000000ff
#define VX_DATA_MASK_FLAGS			0x0f800000
#define VX_DATA_MASK_AO				0xe0000000

#define VX_DATA_GET_TYPE(x)			((x & VX_DATA_MASK_TYPE))
#define VX_DATA_GET_FLAGS(x)		((x & VX_DATA_MASK_FLAGS) >> 23)
#define VX_DATA_GET_AO(x)			((x & VX_DATA_MASK_AO) >> 29)

#define VX_DATA_SET_TYPE(x)			((x & VX_DATA_MASK_TYPE))
#define VX_DATA_SET_FLAGS(x)		((x & VX_DATA_MASK_LOW8) << 23)
#define VX_DATA_SET_AO(x)			((x & VX_DATA_MASK_LOW8) << 29)

// chunk size
#define VX_CHUNK_SIZE_XZ			16
#define VX_CHUNK_SIZE_Y				16

// floats
#define VX_CHUNK_SIZE_XZ_F			16.0f
#define VX_CHUNK_SIZE_Y_F			16.0f

// faster voxel lookups
#define VX_CHUNK_SIZE_XZ_LIM		VX_CHUNK_SIZE_XZ - 1
#define VX_CHUNK_SIZE_Y_LIM			VX_CHUNK_SIZE_Y - 1
#define VX_CHUNK_SIZE_XZ_LIM_F		VX_CHUNK_SIZE_XZ_F - 1.0f
#define VX_CHUNK_SIZE_Y_LIM_F		VX_CHUNK_SIZE_Y_F - 1.0f

#define VX_CHUNK_SIZE_XZ_DIV		1.0f / VX_CHUNK_SIZE_XZ_F
#define VX_CHUNK_SIZE_Y_DIV			1.0f / VX_CHUNK_SIZE_Y_F

// meshing directions -- for threaded meshing
#define VX_CHUNK_MESH_DIR_POS_X		0
#define VX_CHUNK_MESH_DIR_POS_Y		1
#define VX_CHUNK_MESH_DIR_POS_Z		2
#define VX_CHUNK_MESH_DIR_NEG_X		3
#define VX_CHUNK_MESH_DIR_NEG_Y		4
#define VX_CHUNK_MESH_DIR_NEG_Z		5

// remeshing instructions packed into a uint32_t
#define VX_CHUNK_REMESH_POS_X		0x01
#define VX_CHUNK_REMESH_POS_Y		0x02
#define VX_CHUNK_REMESH_POS_Z		0x04
#define VX_CHUNK_REMESH_NEG_X		0x08
#define VX_CHUNK_REMESH_NEG_Y		0x0f
#define VX_CHUNK_REMESH_NEG_Z		0x10

// xz size squared
#define VX_CHUNK_SIZE_XZ_2			VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_XZ
// xz * y
#define VX_CHUNK_SIZE_XZ_x_Y		VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_Y
// x * y * z
#define VX_CHUNK_SIZE_ALLOC			VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_XZ * VX_CHUNK_SIZE_Y

// voxel for loops
#define VX_FOR_BEGIN_XYZ(_x, _y, _z)	for (uint32_t x = 0; x < _x; x++) { \
											for (uint32_t y = 0; y < _y; y++) { \
												for (uint32_t z = 0; z < _z; z++) {

#define VX_FOR_BEGIN(lim)	for (uint32_t x = 0; x < lim; x++) { \
								for (uint32_t y = 0; y < lim; y++) { \
									for (uint32_t z = 0; z < lim; z++) { 

#define VX_FOR_BEGIN_DEF	for (uint32_t x = 0; x < VX_CHUNK_SIZE_XZ; x++) { \
								for (uint32_t y = 0; y < VX_CHUNK_SIZE_Y; y++) { \
									for (uint32_t z = 0; z < VX_CHUNK_SIZE_XZ; z++) { 

#define VX_FOR_END }}}

// set all voxels in chunk to a value
#define VX_SET_CHUNK_VOXELS(value)	for (uint32_t x = 0; x < VX_CHUNK_SIZE_XZ; x++) { \
										for (uint32_t y = 0; y < VX_CHUNK_SIZE_Y; y++) { \
											memset(m_voxels[x][y], value, sizeof(Syn::Voxel) * VX_CHUNK_SIZE_XZ); \
										} \
									}
										

/* Used to extract 8-bit values packed into a 32-bit varaible.
 * Will not be used any longer, voxels need no information on
 * position within chunk.
 */
#define MASK8_32 0xff000000
#define MASK8_24 0x00ff0000
#define MASK8_16 0x0000ff00
#define MASK8_8  0x000000ff

#define VX_GET_X(ui) (float)(((ui) & MASK8_24) >> 16)
#define VX_GET_Y(ui) (float)(((ui) & MASK8_16) >>  8)
#define VX_GET_Z(ui) (float)(((ui) & MASK8_8)  >>  0)

#define VX_SET_XYZ_FLOAT(x, y, z) ((uint8_t(x) & MASK8_8) << 16) | ((uint8_t(y) & MASK8_8) << 8) | (uint8_t(z) & MASK8_8)
#define VX_SET_XYZ_UINT(x, y, z)  ((uint8_t(x) & MASK8_8) << 16) | ((uint8_t(y) & MASK8_8) << 8) | (uint8_t(z) & MASK8_8)


namespace Syn
{

    inline glm::vec3 get_vec3_from_uint32_t(const uint32_t& ui)
    { return glm::vec3(VX_GET_X(ui), VX_GET_Y(ui), VX_GET_Z(ui)); }
    inline uint32_t get_uint32_t_from_vec3(const glm::vec3& v)
    { return VX_SET_XYZ_FLOAT(v.x, v.y, v.z); }
    inline uint32_t get_uint32_t_from_uint8_t_3(uint8_t x, uint8_t y, uint8_t z)
    { return VX_SET_XYZ_UINT(x, y, z); }


	//
	struct Voxel
	{
		// high 3 bits contain ambient occlusion data (0-2)
		// next 5 bits are flags (4-8)
		// low 8 bits contain type (24-31)
		//
		// 0b 1111 1111 0000 0000 0000 0000 1111 1111 
		//    ---  ------                   ---------
		//    AO   flags                      type
		// 
		Voxel() : data(0) {}
		Voxel(uint32_t _data) : data(_data) {}

		uint32_t type() { return VX_DATA_GET_TYPE(data); }
		uint32_t flags() { return VX_DATA_GET_FLAGS(data); }
		uint32_t ao() { return VX_DATA_GET_AO(data); }
		
		// the actual data
		uint32_t data;
	};



}

