#include "MatrixTransformNode.hpp"

namespace starforge {
	MatrixTransformNode::MatrixTransformNode(Node * parent, glm::mat4 m) :
	GroupNode(parent), m_Model(m) {
	} 

	MatrixTransformNode::~MatrixTransformNode() {}
}