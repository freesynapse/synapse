#pragma once


#include <vector>
#include <string>

#include "Synapse/Renderer/Mesh/Mesh.hpp"
#include "Synapse/Renderer/Mesh/MeshDebug.hpp"
#include "Synapse/Renderer/Mesh/MeshShape.hpp"
#include "Synapse/Renderer/Shader/Shader.hpp"
#include "Synapse/Renderer/Camera/Camera.hpp"
#include "Synapse/Renderer/Renderer.hpp"


namespace Syn {


	class MeshCreator
	{
	public:
		friend class MeshDebug;
		friend class MeshShape;

	public:
		// Shape meshes
		
		/* 
		Creates a 'standard' unit cube, 36 vertices, translated by _center, with _diameter sides.
		Position, normal and UV shader attributes as default. 
		*/
		static Ref<MeshShape> createShapeCube(const glm::vec3& _center=glm::vec3(0.0f), 
											  float _diameter=1.0f, 
											  uint32_t _mesh_attrib_flags=MESH_ATTRIB_POSITION|MESH_ATTRIB_NORMAL|MESH_ATTRIB_UV);
		
		/*
		Creates a quad aligned to the x-y-plane with a single z coordinate. If other orientations
		are required, use transformations of the model matrix.
		*/
		static Ref<MeshShape> createShapeQuad(const glm::vec3& _center=glm::vec3(0.0f),
											  float _side=1.0f,
											  uint32_t _mesh_attrib_flags=MESH_ATTRIB_POSITION|MESH_ATTRIB_NORMAL|MESH_ATTRIB_UV);

		/*
		A quad spanning the viewport, in screen coordinates. Requires a shader that can handle
		this (i.e. outputting un-transformed vertex positions). Typically used for rendering
		whole screen framebuffer textures.
		*/
		static Ref<MeshShape> createShapeViewportQuad();

		/*
		Sphere, defined in polar coordinates; vertex positions calculated through number of
		stacks (vertical) and sectors (horizontal) specified.
		*/
		static Ref<MeshShape> createShapeSphere(const glm::vec3& _center=glm::vec3(0.0f),
												float _radius=10.0f,
												uint32_t _stack_count=21,
												uint32_t _sector_count=21,
												uint32_t _mesh_attrib_flags=MESH_ATTRIB_POSITION|MESH_ATTRIB_NORMAL|MESH_ATTRIB_UV);

	public:
		// Debug meshes

		// Cube with the around _center with side +/- _diameter. GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugCube(const glm::vec3& _center = glm::vec3(0.0f), float _diameter = 1.0f, const std::string& _name = "");
		// Sphere with _radius around _center, with longitudinal and latitudinal resoluton of _stack_count and _sector_count, respectively. GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugSphere(const glm::vec3& _center=glm::vec3(0.0f), 
										   		float _radius=10.0f, 
												   uint32_t _stack_count=21, 
												   uint32_t _sector_count=21, 
												   const std::string& _name="");
		// Vector of points, gl_PointSize = _size. GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugPoints(const std::vector<glm::vec3>& _points, float _size, const std::string& _name="");
		// Vector of vertices representing lines, rendered as GL_LINE_STRIP. GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugLineStrip(const std::vector<glm::vec3>& _line_vertices, const std::string& _name = "");
		// Vector of pairs of vertices representing lines, rendered as GL_LINES. GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugLines(const std::vector<glm::vec3>& _vertex_pairs, const std::string& _name = "");
		// A box based on an axis-aligned bounding box (AABB). GL_DYNAMIC_DRAW by default.
		static Ref<MeshDebug> createDebugAABB(const AABB& _aabb, const std::string& _name="");

	public:
		// Accessors
		static Ref<MeshDebug> getMeshDebugPtr(const std::string& _name);
		static Ref<MeshDebug> getMeshPtr(const std::string& _name);



	public:
		// Render all debug meshes; in wireframe mode by default.
		static void renderDebugMeshes(const Ref<Camera>& _camera_ptr, bool _wireframe=true);

		// Renders a single debug mesh; in wireframe mode by default. May be called with reference or name.
		static void renderDebugMesh(const Ref<MeshDebug>& _mesh_ptr, const Ref<Camera>& _camera_ptr, bool _wireframe=true);
		// Renders a single debug mesh; in wireframe mode by default. May be called with reference or name.
		static void renderDebugMesh(const std::string& _mesh_name, const Ref<Camera>& _camera_ptr, bool _wireframe=true)
		{ renderDebugMesh(getMeshDebugPtr(_mesh_name), _camera_ptr, _wireframe); }

		// Clear all mesh references. Note: external references will still keep the object alive.
		static void clearDebugMeshes();


	public:
		// Create a VertexArray reference, given the usual specifications.
		static Ref<VertexArray> createVertexArray(void* _vertices,
												  uint32_t _vertices_size_bytes, 
												  void* _indices, 
												  uint32_t _index_count, 
												  uint32_t _mesh_attrib_flags=MESH_ATTRIB_POSITION,
												  GLenum _usage=GL_STATIC_DRAW);

	private:
		// Helper method for setting up debug meshes.
		static Ref<MeshDebug> createMeshDebug(const std::string& _mesh_name,
										 	  MeshDebugType _type, 
											  void* _vertices, 
											  uint32_t _vertices_size_bytes, 
											  void* _indices, 
											  uint32_t _index_count);

		// Helper method for setting up shape meshes.
		static Ref<MeshShape> createMeshShape(void* _vertices, 
											  uint32_t _vertices_size_bytes, 
											  void* _indices, 
											  uint32_t _index_count, 
											  uint32_t _mesh_attrib_flags=MESH_ATTRIB_POSITION);

		// Setup static shaders used for rendering debug meshes.
		static void createShaders();


	private:
		// Permits automatic sorting of types, dictating which static shader to use.
		static std::multimap<MeshDebugType, Ref<MeshDebug>> s_MeshDebugMap;
		
		// Permits acquisition of created meshes (same Ref:s as in s_debugMap).
		static boost::unordered_map<std::string, Ref<MeshDebug>> s_MeshDebugSearchMap;
		
		static bool s_shadersCreated;
		static uint32_t s_meshIndex;
	};

}