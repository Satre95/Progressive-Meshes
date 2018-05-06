#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cassert>
#include <functional>

struct Vertex {
	Vertex(glm::vec4 _pos = glm::vec4(0, 0, 0, 1.f),
			 glm::vec4 _norm = glm::vec4(0.f),
			 glm::vec4 _color = glm::vec4(1.f, 0.4f, 0.1f, 1.f)) :
			mPos(_pos), mNormal(_norm), mColor(_color),
            mId(sCount++)
			{}
    Vertex(const Vertex & other) :
    mPos(other.mPos), mNormal(other.mNormal),
    mColor(other.mColor), mId(sCount++)
    {}

	glm::vec4 mPos;
	glm::vec4 mNormal;
	glm::vec4 mColor;
	const size_t mId;
	static size_t sCount;
};

struct Face
{
    Face(): mId(sCount++) {
        mIndices[0] = mIndices[1] = mIndices[2] = 0;
    }

	Face(size_t i0, size_t i1, size_t i2): mId(sCount++) {
        mIndices[0] = i0;
        mIndices[1] = i1;
        mIndices[2] = i2;
	}

	Face(const Face & other) : mId(sCount++) {
	    mIndices[0] = other.mIndices[0];
        mIndices[1] = other.mIndices[1];
        mIndices[2] = other.mIndices[2];
	}

	size_t GetIndex(size_t i) const { assert(i < 3); return mIndices[i]; }

	size_t mIndices[3];
    const size_t mId;
    static size_t sCount ;
};


/// Inject Vertex & Face hashing functions into std namespace
namespace std {
    template <> struct hash<Vertex> {
        typedef Vertex argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& v) const {
            return std::hash<size_t>{}(v.mId);
        }
    };

    template <> struct hash<Face> {
        typedef Face argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& f) const  {
            return std::hash<size_t>{}(f.mId);
        }
    };
}

/// Create a custom hashing function for Vertex pointers
struct VertexPtrHash {
    std::size_t operator()(Vertex const * const & vPtr) const noexcept {
        return std::hash<Vertex>{}(*vPtr);
    }
};

/// Create a custom hashing function for Face pointers.
struct FacePtrHash {
    std::size_t operator()(Face const * const & fPtr)const noexcept {
        return std::hash<Face>{}(*fPtr);
    }
};

inline bool operator==(const Vertex & lhs, const Vertex & rhs)  {
    return lhs.mId == rhs.mId;
}

inline bool operator==(const Face & lhs, const Face & rhs) {
    return lhs.mId == rhs.mId;
}