#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cassert>
#include <functional>
#include "Utilities.hpp"


struct Vertex {
	Vertex(glm::vec4 _pos = glm::vec4(0, 0, 0, 1.f),
			 glm::vec4 _norm = glm::vec4(0.f),
			 glm::vec4 _color = glm::vec4(1.f, 0.4f, 0.1f, 1.f)) :
			mPos(_pos), mNormal(_norm), mColor(_color),
            mId(sCount++)
			{}
    Vertex(const Vertex & other) :
    mPos(other.mPos), mNormal(other.mNormal),
    mColor(other.mColor), mId(other.mId)
    {}


    Vertex& operator=(const Vertex & other) {
    	if(mId != other.mId) { //Self assignment check
	    	mPos = other.mPos;
	    	mNormal = other.mNormal;
	    	mColor = other.mColor;
    	}
    	return *this;
    }

	glm::vec4 mPos;
	glm::vec4 mNormal;
	glm::vec4 mColor;
	const size_t mId;
	static size_t sCount;
};


inline bool operator==(const Vertex & lhs, const Vertex & rhs)  {
    return lhs.mId == rhs.mId;
}

struct Face
{
    Face(): mId(sCount++) {
        mVertices[0] = mVertices[1] = mVertices[2] = nullptr;
    }

    Face(Vertex & v0, Vertex & v1, Vertex & v2): mId(sCount++) {
        mVertices[0] = &v0; mVertices[1] = &v1; mVertices[2] = & v2;
    }

    Face(Vertex * v0, Vertex * v1, Vertex * v2): mId(sCount++) {
        mVertices[0] = v0; mVertices[1] = v1; mVertices[2] = v2;
    }


    Face(const Face & other) : mId(other.mId) {
	    mVertices[0] = other.mVertices[0];
        mVertices[1] = other.mVertices[1];
        mVertices[2] = other.mVertices[2];
	}

	Vertex * GetVertex(size_t i) const { assert(i < 3); return mVertices[i]; }

	bool ReplaceVertex(Vertex* oldV, Vertex* newV) {
		for(Vertex* & aVertPtr: mVertices) {
			if (*oldV == *aVertPtr)		
			{
				aVertPtr = newV;
				return true;
			}
		}

		return false;
	}

	Vertex* mVertices[3];
    const size_t mId;
    static size_t sCount ;
};

class Pair {
public:
	
	Pair(Vertex & vStart, Vertex & vEnd):
	v0(&vStart), v1(&vEnd)
	{}

	Pair(Vertex * vStart, Vertex * vEnd):
	v0(vStart), v1(vEnd)
	{}

	Pair(const Pair & other):
	v0(other.v0), v1(other.v1)
	{}

	Vertex CalcOptimal() { 
		return Vertex( ((v0->mPos + v1->mPos)/2.0f),
						((v0->mNormal + v1->mNormal) / 2.0f), 
						((v0->mColor + v1->mColor) / 2.0f) ); 
	}

	Vertex* v0 = nullptr;
	Vertex* v1 = nullptr;
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

    template <> struct hash<std::pair<Vertex*, Vertex*>> {
    	typedef std::pair<Vertex*, Vertex*> argument_type;
    	std::size_t operator() (argument_type const & f) const {
    		std::size_t seed = 0;
    		Utilities::hash_combine(seed, f.first);
    		Utilities::hash_combine(seed, f.second);
    		return seed;
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

inline bool operator==(const Face & lhs, const Face & rhs) {
    return lhs.mId == rhs.mId;
}