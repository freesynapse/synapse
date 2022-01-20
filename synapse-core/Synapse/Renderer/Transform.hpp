#pragma once


#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Synapse/Core.hpp"
#include "Synapse/Renderer/Camera/Camera.hpp"


namespace Syn {


	class Transform
	{
	public:
		Transform() :
			m_matModel(glm::mat4(1.0f)), 
			m_matTranslate(glm::mat4(1.0f)), 
			m_matRotation(glm::mat4(1.0f)), 
			m_matScale(glm::mat4(1.0f)),
			m_quatRotation(glm::quat())
		{}
		~Transform() {}

		inline void updateModelMatrix() { m_matModel = m_matTranslate * m_matRotation * m_matScale; }

		void axisAngleRotation(float _theta_degree, const glm::vec3& _axis)
		{
			float theta_div_2 = glm::radians(_theta_degree);
			float sin_theta_div_2 = sinf(theta_div_2);
			glm::quat q;
			q.w = cosf(theta_div_2);
			q.x = _axis.x * sin_theta_div_2;
			q.y = _axis.y * sin_theta_div_2;
			q.z = _axis.z * sin_theta_div_2;

			q = glm::normalize(q);

			// update rotation
			m_quatRotation = glm::normalize(q * m_quatRotation);

			// update rotation matrix
			m_matRotation = glm::toMat4(m_quatRotation);

			// update model matrix after rotation
			updateModelMatrix();
		}

		inline void reset() { m_matModel = glm::mat4(1.0f); }
		inline void scale(const glm::vec3& _v) { m_matScale = glm::scale(m_matScale, _v); updateModelMatrix(); }
		inline void scale(float _f) { m_matScale = glm::scale(m_matScale, glm::vec3(_f)); updateModelMatrix(); }
		inline void scaleReset(const glm::vec3& _v) { m_matScale = glm::scale(glm::mat4(1.0f), _v); updateModelMatrix(); }
		inline void scaleReset(float _f) { m_matScale = glm::scale(glm::mat4(1.0f), glm::vec3(_f)); updateModelMatrix(); }
		inline void translate(const glm::vec3& _v) { m_matTranslate = glm::translate(m_matTranslate, _v); updateModelMatrix(); }
		inline void translateReset(const glm::vec3& _v) { m_matTranslate = glm::translate(glm::mat4(1.0f), _v); updateModelMatrix(); }

		inline const glm::mat4& getModelMatrix() const { return m_matModel; }
		inline const glm::mat4& getTranslationMatrix() const { return m_matTranslate; }
		inline const glm::mat4& getRotationMatrix() const { return m_matRotation; }
		inline const glm::mat4& getScaleMatrix() const { return m_matScale; }
		inline const glm::quat& getRotationQuaternion() const { return m_quatRotation; }

		inline const void setModelMatrix(const glm::mat4& _m) { m_matModel = _m; }
		inline const void setTranslationMatrix(const glm::mat4& _m) { m_matTranslate = _m; updateModelMatrix(); }
		inline const void setRotationMatrix(const glm::mat4& _m) { m_matRotation = _m; updateModelMatrix(); }
		inline const void setScaleMatrix(const glm::mat4& _m) { m_matScale = _m; updateModelMatrix(); }
		inline const void setRotationQuaternion(const glm::quat& _q) { m_quatRotation = _q; }

	private:
		glm::mat4 m_matModel = glm::mat4(1.0f);
		glm::mat4 m_matTranslate = glm::mat4(1.0f);
		glm::mat4 m_matRotation = glm::mat4(1.0f);
		glm::mat4 m_matScale = glm::mat4(1.0f);

		glm::quat m_quatRotation = glm::quat();

	};


}
