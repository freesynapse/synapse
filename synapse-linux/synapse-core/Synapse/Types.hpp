
#pragma once


#include <glm/glm.hpp>
#include "Synapse/Debug/Log.hpp"


//---------------------------------------------------------------------------------------
// Voxel related macros and inlines
//

// use threaded meshing?
#define VX_THREADED_MESHING
// use even greedier meshing
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

inline glm::vec3 get_vec3_from_uint32_t(const uint32_t& ui)
{ return glm::vec3(VX_GET_X(ui), VX_GET_Y(ui), VX_GET_Z(ui)); }
inline uint32_t get_uint32_t_from_vec3(const glm::vec3& v)
{ return VX_SET_XYZ_FLOAT(v.x, v.y, v.z); }
inline uint32_t get_uint32_t_from_uint8_t_3(uint8_t x, uint8_t y, uint8_t z)
{ return VX_SET_XYZ_UINT(x, y, z); }


namespace Syn {


	class VertexBase
	{
	public:
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
		glm::vec2 uv;

		VertexBase() :
			position(glm::vec4(0.0f)),
			normal(glm::vec3(0.0f)),
			tangent(glm::vec3(0.0f)),
			bitangent(glm::vec3(0.0f)),
			uv(glm::vec2(0.0f))
		{}
	};

	//
	struct Index
	{
		uint32_t i0;
		uint32_t i1;
		uint32_t i2;
	};

	//
	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;

		AABB() :
			min(glm::vec3(std::numeric_limits<float>::max())),
			max(glm::vec3(std::numeric_limits<float>::lowest()))
		{}
		AABB(const glm::vec3& _min, const glm::vec3& _max) :
			min(_min), max(_max)
		{}

		void print(const char* _func="", const char* _msg="AABB")
		{
			// TODO : add the _msg to the actual message string...
			Log::debug_vector(_func, "AABB::min", this->min);
			Log::debug_vector(_func, "AABB::max", this->max);
		}
	};


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


	//
	template<typename T>
	struct LineT
	{
		T p0;
		T p1;

		LineT() : 
			p0(T(0)), p1(T(0)) {}

		LineT(const T& _p0, const T& _p1) :
			p0(_p0), p1(_p1) {}

		float length() { return glm::distance(p0, p1); }

	};
	using Line = LineT<glm::vec3>;


	// Noise parameters
	//
	enum class NoiseType
	{
		Value = 0,
		Simplex,
		Perlin,
		Cellular,
		Cubic
	};
	
	struct NoiseParameters
	{
		int octaveCount;
		float baseFreq;
		float deltaFreq;
		float deltaAmp;
		glm::vec3 offset;

		NoiseParameters(int _octaves, float _freq, float _delta_freq, float _delta_amp, const glm::vec3& _offset) :
			octaveCount(_octaves), baseFreq(_freq), deltaFreq(_delta_freq), deltaAmp(_delta_amp), offset(_offset)
		{}

		NoiseParameters() :
			octaveCount(1), baseFreq(0.01f), deltaFreq(2.0f), deltaAmp(0.5f), offset(glm::vec3(0))
		{}

		void print(const std::string& _name="")
		{
			std::cout << _name << '\n';
			std::cout << "octaveCount = " << octaveCount << '\n';
			std::cout << "baseFreq = " << baseFreq << '\n';
			std::cout << "deltaFreq = " << deltaFreq << '\n';
			std::cout << "deltaAmp = " << deltaAmp << '\n';
			std::cout << "offset = [ " << offset.x << "  " << offset.y << "  " << offset.z << " ]\n";
		}
	};

}

//---------------------------------------------------------------------------------------
// Hashes for various glm types, for use with std::unordered_map<>
//
struct ivec2Hash
{
	size_t operator()(const glm::ivec2& _k) const { return std::hash<int>()(_k.x) ^ std::hash<int>()(_k.y); }
	bool operator()(const glm::ivec2& _a, const glm::ivec2& _b) const { return _a.x == _b.x && _a.y == _b.y; }
};
struct vec2Hash
{
	size_t operator()(const glm::vec2& _k) const { return std::hash<float>()(_k.x) ^ std::hash<float>()(_k.y); }
	bool operator()(const glm::vec2& _a, const glm::vec2& _b) const { return _a.x == _b.x && _a.y == _b.y; }
};
struct ivec3Hash
{
	size_t operator()(const glm::ivec3& _k) const { return std::hash<int>()(_k.x) ^ (std::hash<int>()(_k.y) << 1) ^ std::hash<int>()(_k.z); }
	bool operator()(const glm::ivec3& _a, const glm::ivec3& _b) const { return _a.x == _b.x && _a.y == _b.y && _a.z == _b.z; }
};
struct vec3Hash
{
	size_t operator()(const glm::vec3& _k) const { return std::hash<float>()(_k.x) ^ (std::hash<float>()(_k.y) << 1) ^ std::hash<float>()(_k.z); }
	bool operator()(const glm::vec3& _a, const glm::vec3& _b) const { return _a.x == _b.x && _a.y == _b.y && _a.z == _b.z; }
};


//---------------------------------------------------------------------------------------
// Compare functions for glm types
//

// priority of x, y and z, respectively
struct ivec3CompareX { bool operator()(const glm::ivec3& _lhs, const glm::ivec3& _rhs) const { return std::tie(_lhs.y, _lhs.z) < std::tie(_rhs.y, _rhs.z); } };
struct ivec3CompareY { bool operator()(const glm::ivec3& _lhs, const glm::ivec3& _rhs) const { return std::tie(_lhs.x, _lhs.z) < std::tie(_rhs.x, _rhs.z); } };
struct ivec3CompareZ { bool operator()(const glm::ivec3& _lhs, const glm::ivec3& _rhs) const { return std::tie(_lhs.x, _lhs.y) < std::tie(_rhs.y, _rhs.z); } };

// priority of x, y and z, respectively
struct vec3CompareX { bool operator()(const glm::vec3& _lhs, const glm::vec3& _rhs) const { return std::tie(_lhs.y, _lhs.z) < std::tie(_rhs.y, _rhs.z); } };
struct vec3CompareY { bool operator()(const glm::vec3& _lhs, const glm::vec3& _rhs) const { return std::tie(_lhs.x, _lhs.z) < std::tie(_rhs.x, _rhs.z); } };
struct vec3CompareZ { bool operator()(const glm::vec3& _lhs, const glm::vec3& _rhs) const { return std::tie(_lhs.x, _lhs.y) < std::tie(_rhs.y, _rhs.z); } };

