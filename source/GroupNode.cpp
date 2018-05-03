#include "GroupNode.hpp"

namespace starforge {
	GroupNode::GroupNode(Node * parent) : Node(parent) {
	}

	GroupNode::~GroupNode() {
		for (Node * aNode : m_children)
			delete aNode;
		m_children.clear();
	}

	void GroupNode::AddChild(Node * child) {
		m_children.push_back(child);
	}

	void GroupNode::RemoveChild(size_t i) {
		if (i >= m_children.size()) return;
		m_children.erase(m_children.begin() + i);
	}

	const std::vector<Node *> & GroupNode::GetChildren() const {
		return m_children;
	}
}