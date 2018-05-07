#pragma once

#include <glad/glad.h>
#include "RenderDevice.hpp"

namespace starforge {
    class OpenGLRasterState : public RasterState {
    public:

        OpenGLRasterState(bool _cullEnabled = false, Winding _frontFace = WINDING_CCW, Face _cullFace = FACE_BACK,
                          RasterMode _rasterMode = RASTERMODE_FILL) :
                id(count++) {
            static const GLenum front_face_map[] = {GL_CW, GL_CCW};
            static const GLenum cull_face_map[] = {GL_FRONT, GL_BACK, GL_FRONT_AND_BACK};
            static const GLenum raster_mode_map[] = {GL_POINT, GL_LINE, GL_FILL};

            cullEnabled = _cullEnabled;
            frontFace = front_face_map[_frontFace];
            cullFace = cull_face_map[_cullFace];
            polygonMode = raster_mode_map[_rasterMode];
        }

        bool operator==(const RasterState &obj) const override {
            return id == (static_cast<const OpenGLRasterState &>(obj).id);
        };


        bool cullEnabled;
        GLenum frontFace;
        GLenum cullFace;
        GLenum polygonMode;


    private:
        const size_t id;
        static size_t count;
    };

    class OpenGLDepthStencilState : public DepthStencilState {
    public:

        OpenGLDepthStencilState(
                bool _depthEnabled = true,
                bool _depthWriteEnabled = true,
                float _depthNear = 0,
                float _depthFar = 1,
                Compare _depthCompare = COMPARE_LESS,

                bool _frontFaceStencilEnabled = false,
                Compare _frontFaceStencilCompare = COMPARE_ALWAYS,
                StencilAction _frontFaceStencilFail = STENCIL_KEEP,
                StencilAction _frontFaceStencilPass = STENCIL_KEEP,
                StencilAction _frontFaceDepthFail = STENCIL_KEEP,
                int _frontFaceRef = 0,
                unsigned int _frontFaceReadMask = 0xFFFFFFFF,
                unsigned int _frontFaceWriteMask = 0xFFFFFFFF,

                bool _backFaceStencilEnabled = true,
                Compare _backFaceStencilCompare = COMPARE_ALWAYS,
                StencilAction _backFaceStencilFail = STENCIL_DECR,
                StencilAction _backFaceStencilPass = STENCIL_KEEP,
                StencilAction _backFaceDepthFail = STENCIL_DECR,
                int _backFaceRef = 0,
                unsigned int _backFaceReadMask = 0xFFFFFFFF,
                unsigned int _backFaceWriteMask = 0xFFFFFFFF) : id(count++) {
            static const GLenum compare_map[] = {GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL,
                                                 GL_GEQUAL, GL_ALWAYS};
            static const GLenum stencil_map[] = {GL_KEEP, GL_ZERO, GL_REPLACE, GL_INCR, GL_INCR_WRAP, GL_DECR,
                                                 GL_DECR_WRAP, GL_INVERT};

            depthEnabled = _depthEnabled;
            depthWriteEnabled = _depthWriteEnabled;
            depthNear = _depthNear;
            depthFar = _depthFar;
            depthFunc = compare_map[_depthCompare];

            frontFaceStencilEnabled = _frontFaceStencilEnabled;
            frontStencilFunc = compare_map[_frontFaceStencilCompare];
            frontFaceStencilFail = stencil_map[_frontFaceStencilFail];
            frontFaceStencilPass = stencil_map[_frontFaceStencilPass];
            frontFaceDepthFail = stencil_map[_frontFaceDepthFail];
            frontFaceRef = _frontFaceRef;
            frontFaceReadMask = _frontFaceReadMask;
            frontFaceWriteMask = _frontFaceWriteMask;

            backFaceStencilEnabled = _backFaceStencilEnabled;
            backStencilFunc = compare_map[_backFaceStencilCompare];
            backFaceStencilFail = stencil_map[_backFaceStencilFail];
            backFaceStencilPass = stencil_map[_backFaceStencilPass];
            backFaceDepthFail = stencil_map[_backFaceDepthFail];
            backFaceRef = _backFaceRef;
            backFaceReadMask = _backFaceReadMask;
            backFaceWriteMask = _backFaceWriteMask;
        }

        bool operator==(const DepthStencilState &obj) const override {
            return id == (static_cast<const OpenGLDepthStencilState &>(obj).id);
        };

        bool depthEnabled;
        bool depthWriteEnabled;
        float depthNear;
        float depthFar;
        GLenum depthFunc;

        bool frontFaceStencilEnabled;
        GLenum frontStencilFunc;
        GLenum frontFaceStencilFail;
        GLenum frontFaceStencilPass;
        GLenum frontFaceDepthFail;
        GLint frontFaceRef;
        GLuint frontFaceReadMask;
        GLuint frontFaceWriteMask;

        bool backFaceStencilEnabled;
        GLenum backStencilFunc;
        GLenum backFaceStencilFail;
        GLenum backFaceStencilPass;
        GLenum backFaceDepthFail;
        GLint backFaceRef;
        GLuint backFaceReadMask;
        GLuint backFaceWriteMask;

    private:
        const size_t id;
        static size_t count;

    };
}