#pragma once

#include <glm/matrix.hpp>
#include "GroupNode.hpp"

namespace starforge {
	class MatrixTransformNode: public GroupNode
	{
	public:
		MatrixTransformNode(Node * parent = nullptr, glm::mat4 m = glm::mat4(1.0f));
		virtual ~MatrixTransformNode();
		
		void SetMatrix( glm::mat4 mat) { m_Model = mat;}
		const glm::mat4 & GetMatrix() const { return m_Model; }

	protected:
		glm::mat4 m_Model;
	};
}