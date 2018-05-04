#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cassert>

struct Vertex {
	Vertex(glm::vec4 _pos = glm::vec4(0, 0, 0, 1.f),
			 glm::vec4 _norm = glm::vec4(0.f),
			 glm::vec4 _color = glm::vec4(1.f, 0.4f, 0.1f, 1.f)) :
			mPos(_pos), mNormal(_norm), mColor(_color)
			{}


	glm::vec4 mPos;
	glm::vec4 mNormal;
	glm::vec4 mColor;
};

class Face
{
public:
	Face(size_t i0, size_t i1, size_t i2) {
		indices[0] = i0;
		indices[1] = i1;
		indices[2] = i2;
	}

	size_t GetIndex(size_t i) { assert(i < 3); return indices[i]; }
	
private:
	size_t indices[3];

};