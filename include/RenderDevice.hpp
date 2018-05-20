#pragma once
#include <string>
#include <glm/matrix.hpp>

namespace starforge {

    class Model;
/// Encapsulates a vertex shader defintion
class VertexShader
{
public:
    virtual ~VertexShader() {}
protected:
    VertexShader() {}
};

/// Encapsulates a pixel (fragment) shader definition
class PixelShader
{
public:
    virtual ~PixelShader() {}
protected:
    PixelShader() {}
};

/// Encapsulates a shader pipeline uniform parameter.
class PipelineParam
{
public:
    virtual ~PipelineParam() {}

    virtual void SetAsBool(bool value) = 0;

    virtual void SetAsInt(int value) = 0;

    virtual void SetAsFloat(float value) = 0;

    virtual void SetAsMat4(const float *value) = 0;

    virtual void SetAsMat3(const float *value) = 0;

    virtual void SetAsVec3(const float * value) = 0;

    virtual void SetAsVec4(const float * value) = 0;

    virtual void SetAsIntArray(int count, const int *values) = 0;

    virtual void SetAsFloatArray(int count, const float *values) = 0;

    virtual void SetAsMat4Array(int count, const float *values) = 0;
protected:
    PipelineParam() {}
};

/// Encapsulates a shader pipeline
class Pipeline
{
public:
    virtual ~Pipeline() {}
    virtual bool operator==( const Pipeline &) const = 0;
    Pipeline & operator=(const Pipeline &) = delete;
    virtual PipelineParam *GetParam(const char *name) = 0;

protected:
    Pipeline() {}
};

// Encapsulates a vertex buffer
class VertexBuffer
{
public:
    virtual ~VertexBuffer() {}
    VertexBuffer(const VertexBuffer &) = delete;
    VertexBuffer & operator=(const VertexBuffer &) = delete;
    virtual bool operator ==(const VertexBuffer & obj) const = 0;

protected:
    VertexBuffer() {}
};

/// Encapsulates a vertex buffer semantic description
class VertexDescription
{
public:
    virtual ~VertexDescription() {}
    virtual unsigned int NumElements() = 0;
    virtual bool operator==(const VertexDescription & obj) const = 0;

protected:

    VertexDescription() {}
};

/// Encapsulates a collection of vertex buffers and their semantic descriptions
class VertexArray
{
public:
    virtual ~VertexArray() {}
    VertexArray(const VertexArray &) = delete;
    VertexArray & operator=(const VertexArray &) = delete;
    virtual bool operator ==(const VertexArray & obj) const = 0;

protected:
    VertexArray() {}
};

/// Encapsulates an index buffer
class IndexBuffer
{
public:
    virtual ~IndexBuffer() {}
    IndexBuffer(const IndexBuffer &) = delete;
    IndexBuffer& operator=(const IndexBuffer &) = delete;
    virtual bool operator ==(const IndexBuffer & obj) const = 0;

protected:
    IndexBuffer() {}
};

/// Encapsulates a 2D texture
class Texture2D
{
public:
    virtual ~Texture2D() {}
    Texture2D(const Texture2D &) = delete;
    Texture2D & operator=(const Texture2D &) = delete;

protected:
    Texture2D() {}
};

/// Describes a vertex element's type
enum VertexElementType
{
    VERTEXELEMENTTYPE_BYTE = 0,
    VERTEXELEMENTTYPE_SHORT,
    VERTEXELEMENTTYPE_INT,

    VERTEXELEMENTTYPE_UNSIGNED_BYTE,
    VERTEXELEMENTTYPE_UNSIGNED_SHORT,
    VERTEXELEMENTTYPE_UNSIGNED_INT,

    VERTEXELEMENTTYPE_BYTE_NORMALIZE,
    VERTEXELEMENTTYPE_SHORT_NORMALIZE,
    VERTEXELEMENTTYPE_INT_NORMALIZE,

    VERTEXELEMENTTYPE_UNSIGNED_BYTE_NORMALIZE,
    VERTEXELEMENTTYPE_UNSIGNED_SHORT_NORMALIZE,
    VERTEXELEMENTTYPE_UNSIGNED_INT_NORMALIZE,

    VERTEXELEMENTTYPE_HALF_FLOAT,
    VERTEXELEMENTTYPE_FLOAT,
    VERTEXELEMENTTYPE_DOUBLE
};

/// Describes a vertex element within a vertex buffer
struct VertexElement
{
    VertexElement() {}
    VertexElement(unsigned int indexIn, VertexElementType typeIn, int sizeIn, int strideIn, long long offsetIn) :
            index(indexIn), type(typeIn), size(sizeIn), stride(strideIn), offset(offsetIn) {}
    /// location binding for vertex element;
    unsigned int index;
    /// type of vertex element
    VertexElementType type;
    /// number of components
    int size;
    /// number of bytes between each successive element (leave zero for this to be assumed to be size times size of type)
    int stride;
    /// offset where first occurrence of this vertex element resides in the buffer
    long long offset;
};

/// Encapsulates the rasterizer state
class RasterState
{
public:
    virtual ~RasterState() {}
    virtual bool operator ==(const RasterState & obj) const = 0;

protected:
    RasterState() {}
};

enum Winding
{
    WINDING_CW = 0,
    WINDING_CCW,
    WINDING_MAX
};

enum Face
{
    FACE_FRONT = 0,
    FACE_BACK,
    FACE_FRONT_AND_BACK,
    FACE_MAX
};

enum RasterMode
{
    RASTERMODE_POINT = 0,
    RASTERMODE_LINE,
    RASTERMODE_FILL,
    RASTERMODE_MAX
};

/// Encapsulates the depth/stencil state
class DepthStencilState
{
public:
    virtual ~DepthStencilState() {}
    virtual bool operator ==(const DepthStencilState & obj) const = 0;

protected:
    DepthStencilState() {}
};

enum Compare
{
    /// Test comparison never passes
            COMPARE_NEVER = 0,

    /// Test comparison passes if the incoming value is less than the stored value.
            COMPARE_LESS,

    /// Test comparison passes if the incoming value is equal to the stored value.
            COMPARE_EQUAL,

    /// Test comparison passes if the incoming value is less than or equal to the stored value.
            COMPARE_LEQUAL,

    /// Test comparison passes if the incoming value is greater than the stored value.
            COMPARE_GREATER,

    /// Test comparison passes if the incoming value is not equal to the stored value.
            COMPARE_NOTEQUAL,

    /// Test comparison passes if the incoming value is greater than or equal to the stored value.
            COMPARE_GEQUAL,

    /// Test comparison always passes.
            COMPARE_ALWAYS,

    COMPARE_MAX
};

enum StencilAction
{
    /// Keeps the current value.
            STENCIL_KEEP = 0,

    /// Sets the stencil buffer to zero.
            STENCIL_ZERO,

    /// Sets the stencil buffer to the reference value masked with the write mask.
            STENCIL_REPLACE,

    /// Increments the current stencil buffer value and clamps to maximum unsigned value.
            STENCIL_INCR,

    /// Increments the current stencil buffer value and wraps the stencil buffer to zero when passing the maximum representable unsigned value.
            STENCIL_INCR_WRAP,

    /// Decrements the current stencil buffer value and clamps to zero.
            STENCIL_DECR,

    /// Decrements the current stencil buffer value and wraps the stencil buffer value to the maximum unsigned value.
            STENCIL_DECR_WRAP,

    /// Bitwise invert of the current stencil buffer value.
            STENCIL_INVERT,

    STENCIL_MAX
};

/// Encapsulates the render device API.
class RenderDevice
{
public:

    /// virtual destructor to ensure subclasses have a virtual destructor
    virtual ~RenderDevice() {}

    /// Create a vertex shader from the supplied code; code is assumed to be GLSL for now.
    virtual VertexShader *CreateVertexShader(const char *code) = 0;

    /// Destroy a vertex shader
    virtual void DestroyVertexShader(VertexShader *vertexShader) = 0;

    /// Create a pixel shader from the supplied code; code is assumed to be GLSL for now.
    virtual PixelShader *CreatePixelShader(const char *code) = 0;

    /// Destroy a pixel shader
    virtual void DestroyPixelShader(PixelShader *pixelShader) = 0;

    /// Create a linked shader pipeline given a vertex and pixel shader
    virtual Pipeline *CreatePipeline(VertexShader *vertexShader, PixelShader *pixelShader) = 0;

    /// Destroy a shader pipeline
    virtual void DestroyPipeline(Pipeline *pipeline) = 0;

    /// Set a shader pipeline as active for subsequent draw commands
    virtual void SetPipeline(Pipeline *pipeline) = 0;

    /// Create a vertex buffer
    virtual VertexBuffer *CreateVertexBuffer(long long size, const void *data = nullptr) = 0;

    /// Destroy a vertex buffer
    virtual void DestroyVertexBuffer(VertexBuffer *vertexBuffer) = 0;

    /// Fill an existing vertex buffer with new data.
    virtual void FillVertexBuffer(VertexBuffer * vertexBuffer, long long size, const void * data) = 0;
    
    /// Create a vertex description given an array of VertexElement structures
    virtual VertexDescription *CreateVertexDescription(unsigned int numVertexElements, const VertexElement *vertexElements) = 0;

    /// Destroy a vertex description
    virtual void DestroyVertexDescription(VertexDescription *vertexDescription) = 0;

    /// Create a vertex array given an array of vertex buffers and associated vertex descriptions; the arrays must be the same size.
    virtual VertexArray *CreateVertexArray(unsigned int numVertexBuffers, VertexBuffer **vertexBuffers, VertexDescription **vertexDescriptions) = 0;

    /// Destroy a vertex array
    virtual void DestroyVertexArray(VertexArray *vertexArray) = 0;

    /// Set a vertex array as active for subsequent draw commands
    virtual void SetVertexArray(VertexArray *vertexArray) = 0;

    /// Create an index buffer
    virtual IndexBuffer *CreateIndexBuffer(long long size, const void *data = nullptr) = 0;

    /// Destroy an index buffer
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer) = 0;

    /// Fill an existing vertex buffer with new data.
    virtual void FillIndexBuffer(IndexBuffer * vertexBuffer, long long size, const void * data) = 0;
    
    /// Set an index buffer as active for subsequent draw commands
    virtual void SetIndexBuffer(IndexBuffer *indexBuffer) = 0;

    /** Create a 2D texture.
     *
     * Data is assumed to consist of 32-bit pixel values where
     * 8 bits are used for each of the red, green, and blue
     * components, from lowest to highest byte order. The
     * most significant byte is ignored.
     */
    virtual Texture2D *CreateTexture2D(int width, int height, const void *data = nullptr) = 0;

    /// Destroy a 2D texture
    virtual void DestroyTexture2D(Texture2D *texture2D) = 0;

    /// Set a 2D texture as active on a slot for subsequent draw commands
    virtual void SetTexture2D(unsigned int slot, Texture2D *texture2D) = 0;

    /// Create a raster state.
    virtual RasterState *CreateRasterState(bool cullEnabled = true, Winding frontFace = WINDING_CCW, Face cullFace = FACE_BACK, RasterMode rasterMode = RASTERMODE_FILL) = 0;

    /// Destroy a raster state.
    virtual void DestroyRasterState(RasterState *rasterState) = 0;

    /// Set a raster state for subsequent draw commands
    virtual void SetRasterState(RasterState *rasterState) = 0;

    /// Create a depth/stencil state.
    virtual DepthStencilState *CreateDepthStencilState(bool depthEnabled = true,
                                                       bool depthWriteEnabled = true, float depthNear = 0, float depthFar = 1,
                                                       Compare depthCompare = COMPARE_LESS, bool frontFaceStencilEnabled = false,
                                                       Compare frontFaceStencilCompare = COMPARE_ALWAYS,
                                                       StencilAction frontFaceStencilFail = STENCIL_KEEP,
                                                       StencilAction frontFaceStencilPass = STENCIL_KEEP,
                                                       StencilAction frontFaceDepthFail = STENCIL_KEEP,
                                                       int frontFaceRef = 0, unsigned int frontFaceReadMask = 0xFFFFFFFF,
                                                       unsigned int frontFaceWriteMask = 0xFFFFFFFF,
                                                       bool backFaceStencilEnabled = false,
                                                       Compare backFaceStencilCompare = COMPARE_ALWAYS,
                                                       StencilAction backFaceStencilFail = STENCIL_KEEP,
                                                       StencilAction backFaceStencilPass = STENCIL_KEEP,
                                                       StencilAction backFaceDepthFail = STENCIL_KEEP,
                                                       int backFaceRef = 0, unsigned int backFaceReadMask = 0xFFFFFFFF,
                                                       unsigned int backFaceWriteMask = 0xFFFFFFFF) = 0;

    /// Destroy a depth/stencil state.
    virtual void DestroyDepthStencilState(DepthStencilState *depthStencilState) = 0;

    /// Set a depth/stencil state for subsequent draw commands
    virtual void SetDepthStencilState(DepthStencilState *depthStencilState) = 0;

    virtual void SetPointSize(float pSize) = 0;

    /// Clear the default render target's color buffer, depth buffer, and stencil buffer to the specified values
    virtual void Clear(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f, float depth = 1.0f, int stencil = 0) = 0;

    /// Draw a collection of triangles using the currently active shader pipeline and vertex array data
    virtual void DrawTriangles(int offset, int count) = 0;

    /**
     * Draw a collection of triangles using the currently active shader pipeline, vertex array data,
     * and index buffer
     */
    virtual void DrawTrianglesIndexed32(long long offset, int count) = 0;

    /// Draw a collection of points using the currently active shader pipeline & vertex array data
    virtual void DrawPoints(long long offset, int count) = 0;

    /// Draw a collection of points as a line strip using the currently active shader pipeline & vertex array data
    virtual void DrawLineStrip(long long offset, int count) = 0;

    /// Getter for the default pipeline
    virtual Pipeline * GetDefaultPipeline() = 0;

    /// Sets the default pipeline
    virtual void BindDefaultPipeline() = 0;

    ///Draws the give model
    virtual void DrawModel(Model & aModel, glm::mat4 & arcball, glm::mat4 & view, glm::mat4 & projection) = 0;
};

/// Creates a RenderDevice. Note: Always initilize the platform BEFORE calling this fn!
RenderDevice *CreateRenderDevice();

/// Destroys a RenderDevice. Note: Always destroy the render device BEFORE terminating the platform.
void DestroyRenderDevice(RenderDevice *renderDevice);
}
