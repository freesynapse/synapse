#pragma once


#include "Mesh.h"


namespace Syn {


	class MeshShape : public Mesh
	{
	public:
		MeshShape() {}
		~MeshShape() {}

		void render(const Ref<Shader>& _shader_ptr) override
		{
			_shader_ptr->setMatrix4fv("u_model_matrix", m_transform.getModelMatrix());
			m_vertexArray->bind();
			Renderer::drawIndexed(m_vertexArray->getIndexCount(), true, GL_TRIANGLES);
		}

	};


}