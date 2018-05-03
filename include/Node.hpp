#pragma once

#include <string>

namespace starforge {

	class Node
	{
	public:
		virtual ~Node() {}

		unsigned int GetId() const { return m_id; }
		
	protected:
		//Node is not directly instantiable
		Node(Node * parent = nullptr): m_parent(parent), m_id(nodeCount++) {}
		Node * m_parent = nullptr;
	private: 
		const unsigned int m_id;
		static unsigned int nodeCount;
	};
}