
#pragma once


#include <glm/glm.hpp>
#include <type_traits>

#include "./Debug/Log.hpp"


//---------------------------------------------------------------------------------------
// Return value of enum class
template<typename T>
constexpr auto enum_val(T val) noexcept
{ 
    return static_cast<std::underlying_type_t<T>>(val);
}


namespace Syn 
{
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
	float baseAmp;
	float deltaAmp;
	glm::vec3 offset;

	NoiseParameters(int _octaves, float _freq, float _delta_freq, float _amp, float _delta_amp, const glm::vec3& _offset) :
	    octaveCount(_octaves), baseFreq(_freq), deltaFreq(_delta_freq), baseAmp(_amp), deltaAmp(_delta_amp), offset(_offset)
	{}

	NoiseParameters() :
	    octaveCount(1), baseFreq(0.01f), deltaFreq(2.0f), baseAmp(1.0f), deltaAmp(0.5f), offset(glm::vec3(0))
	{}

	void print(const std::string& _name="")
	{
	    std::cout << _name << '\n';
	    std::cout << "octaveCount = " << octaveCount << '\n';
	    std::cout << "baseFreq = " << baseFreq << '\n';
	    std::cout << "deltaFreq = " << deltaFreq << '\n';
	    std::cout << "baseAmp = " << baseAmp << '\n';
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

