#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGLRenderDevice.hpp"
#include "Utilities.hpp"
#include "Model.hpp"

namespace starforge {

    size_t OpenGLVertexDescription::count = 0;
    size_t OpenGLDepthStencilState::count = 0;
    size_t OpenGLRasterState::count = 0;

    PipelineParam *OpenGLPipeline::GetParam(const char *name) {
        auto const &iter = paramsByName.find(name);
        if (iter == paramsByName.end()) {
            int location = glGetUniformLocation(shaderProgram, name);
            if (location < 0) return nullptr;
            OpenGLPipelineParam *param = new OpenGLPipelineParam(this, location);
            paramsByName.insert(iter, std::make_pair(name, param));
            return param;
        }
        return iter->second;
    }

    class OpenGLTexture2D : public Texture2D {
    public:

        OpenGLTexture2D(int width, int height, const void *data = nullptr) {
            glActiveTexture(GL_TEXTURE0);
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        ~OpenGLTexture2D() override {
            glDeleteTextures(1, &texture);
        }

        unsigned int texture = 0;
    };

    OpenGLRenderDevice::OpenGLRenderDevice() {
        m_DefaultRasterState = dynamic_cast<OpenGLRasterState *>(CreateRasterState());
        SetRasterState(m_DefaultRasterState);

        m_DefaultDepthStencilState = dynamic_cast<OpenGLDepthStencilState *>(CreateDepthStencilState());
        SetDepthStencilState(m_DefaultDepthStencilState);

        auto vertexShader = CreateVertexShader(g_defaultVertexShaderSource);
        auto pixelShader = CreatePixelShader(g_defaultPixelShaderSource);
        m_defaultPipeline = dynamic_cast<OpenGLPipeline *>(CreatePipeline(vertexShader, pixelShader));
        DestroyVertexShader(vertexShader);
        DestroyPixelShader(pixelShader);
    }

    OpenGLRenderDevice::~OpenGLRenderDevice() {
        for (auto &aDState: m_depthStates) Utilities::Safe_Delete(aDState);
        m_depthStates.clear();
        for (auto &aRasterState: m_rasterStates) Utilities::Safe_Delete(aRasterState);
        m_rasterStates.clear();
        for (auto &aPipe: m_pipelines) Utilities::Safe_Delete(aPipe);
        m_pipelines.clear();
        for (auto &aBuf: m_IBOs) Utilities::Safe_Delete(aBuf);
        m_IBOs.clear();
        for (auto &aBuf: m_VBOs) Utilities::Safe_Delete(aBuf);
        m_VBOs.clear();
        for (auto &aVAO: m_VAOs) Utilities::Safe_Delete(aVAO);
        m_VAOs.clear();
        for (auto &aVD: m_vDescriptions) Utilities::Safe_Delete(aVD);
        m_vDescriptions.clear();

        m_RasterState = m_DefaultRasterState = nullptr;
        m_DepthStencilState = m_DefaultDepthStencilState = nullptr;
        m_defaultPipeline = nullptr;
    }

    VertexShader *OpenGLRenderDevice::CreateVertexShader(const char *code) {
        return new OpenGLVertexShader(code);
    }

    void OpenGLRenderDevice::DestroyVertexShader(VertexShader *vertexShader) {
        Utilities::Safe_Delete(vertexShader);
    }

    PixelShader *OpenGLRenderDevice::CreatePixelShader(const char *code) {
        return new OpenGLPixelShader(code);
    }

    void OpenGLRenderDevice::DestroyPixelShader(PixelShader *pixelShader) {
        Utilities::Safe_Delete(pixelShader);
    }

    Pipeline *OpenGLRenderDevice::CreatePipeline(VertexShader *vertexShader, PixelShader *pixelShader) {
        m_pipelines.push_back(new OpenGLPipeline(reinterpret_cast<OpenGLVertexShader *>(vertexShader),
                                                 reinterpret_cast<OpenGLPixelShader *>(pixelShader)));
        return m_pipelines.back();
    }

    void OpenGLRenderDevice::DestroyPipeline(Pipeline *pipeline) {
        if (pipeline) {
            m_pipelines.erase(
                    std::remove_if(m_pipelines.begin(), m_pipelines.end(),
                                   [pipeline](OpenGLPipeline *pipe) { return *pipe == *pipeline; }),
                    m_pipelines.end()
            );

            Utilities::Safe_Delete(pipeline);
        }
    }

    void OpenGLRenderDevice::SetPipeline(Pipeline *pipeline) {
        glUseProgram(reinterpret_cast<OpenGLPipeline *>(pipeline)->shaderProgram);
    }

    VertexBuffer *OpenGLRenderDevice::CreateVertexBuffer(long long size, const void *data) {
        m_VBOs.push_back(new OpenGLVertexBuffer(size, data));
        return m_VBOs.back();
    }

    void OpenGLRenderDevice::DestroyVertexBuffer(VertexBuffer *vertexBuffer) {
        if (vertexBuffer) {
            m_VBOs.erase(
                    std::remove_if(m_VBOs.begin(), m_VBOs.end(),
                                   [vertexBuffer](OpenGLVertexBuffer *buf) { return *buf == *vertexBuffer; }),
                    m_VBOs.end()
            );
            Utilities::Safe_Delete(vertexBuffer);
        }
    }

    VertexDescription *
    OpenGLRenderDevice::CreateVertexDescription(unsigned int numVertexElements, const VertexElement *vertexElements) {
        m_vDescriptions.push_back(new OpenGLVertexDescription(numVertexElements, vertexElements));
        return m_vDescriptions.back();
    }

    void OpenGLRenderDevice::DestroyVertexDescription(VertexDescription *vertexDescription) {
        if (vertexDescription) {
            m_vDescriptions.erase(
                    std::remove_if(m_vDescriptions.begin(), m_vDescriptions.end(),
                                   [vertexDescription](OpenGLVertexDescription *vd) {
                                       return *vd == *vertexDescription;
                                   }
                    ),
                    m_vDescriptions.end()
            );
            Utilities::Safe_Delete(vertexDescription);
        }
    }

    VertexArray *OpenGLRenderDevice::CreateVertexArray(unsigned int numVertexBuffers, VertexBuffer **vertexBuffers,
                                                       VertexDescription **vertexDescriptions) {
        m_VAOs.push_back(new OpenGLVertexArray(numVertexBuffers, vertexBuffers, vertexDescriptions));
        return m_VAOs.back();
    }

    void OpenGLRenderDevice::DestroyVertexArray(VertexArray *vertexArray) {
        if (vertexArray) {
            m_VAOs.erase(
                    std::remove_if(m_VAOs.begin(), m_VAOs.end(),
                                   [vertexArray](OpenGLVertexArray *arr) { return *arr == *vertexArray; }
                    ),
                    m_VAOs.end()
            );
            Utilities::Safe_Delete(vertexArray);
        }
    }

    void OpenGLRenderDevice::SetVertexArray(VertexArray *vertexArray) {
        glBindVertexArray(reinterpret_cast<OpenGLVertexArray *>(vertexArray)->VAO);
    }

    IndexBuffer *OpenGLRenderDevice::CreateIndexBuffer(long long size, const void *data) {
        m_IBOs.push_back(new OpenGLIndexBuffer(size, data));
        return m_IBOs.back();
    }

    void OpenGLRenderDevice::DestroyIndexBuffer(IndexBuffer *indexBuffer) {
        if (indexBuffer) {
            m_IBOs.erase(
                    std::remove_if(m_IBOs.begin(), m_IBOs.end(),
                                   [indexBuffer](OpenGLIndexBuffer *buf) { return *buf == *indexBuffer; }),
                    m_IBOs.end()
            );
            Utilities::Safe_Delete(indexBuffer);
        }
    }

    void OpenGLRenderDevice::SetIndexBuffer(IndexBuffer *indexBuffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, reinterpret_cast<OpenGLIndexBuffer *>(indexBuffer)->IBO);
    }

    Texture2D *OpenGLRenderDevice::CreateTexture2D(int width, int height, const void *data) {
        return new OpenGLTexture2D(width, height, data);
    }

    void OpenGLRenderDevice::DestroyTexture2D(Texture2D *texture2D) {
        Utilities::Safe_Delete(texture2D);
    }

    void OpenGLRenderDevice::SetTexture2D(unsigned int slot, Texture2D *texture2D) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, texture2D ? reinterpret_cast<OpenGLTexture2D *>(texture2D)->texture : 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    RasterState *
    OpenGLRenderDevice::CreateRasterState(bool cullEnabled, Winding frontFace, Face cullFace, RasterMode rasterMode) {
        m_rasterStates.push_back(new OpenGLRasterState(cullEnabled, frontFace, cullFace, rasterMode));
        return m_rasterStates.back();
    }

    void OpenGLRenderDevice::DestroyRasterState(RasterState *rasterState) {
        if (rasterState) {
            m_rasterStates.erase(
                    std::remove_if(m_rasterStates.begin(), m_rasterStates.end(),
                                   [rasterState](RasterState *rState) { return *rState == *rasterState; }),
                    m_rasterStates.end()
            );
            Utilities::Safe_Delete(rasterState);
        }
    }

    void OpenGLRenderDevice::SetRasterState(RasterState *rasterState) {
        RasterState *oldRasterState = m_RasterState;

        if (rasterState)
            m_RasterState = dynamic_cast<OpenGLRasterState *>(rasterState);
        else
            m_RasterState = m_DefaultRasterState;

        if (m_RasterState != oldRasterState) {
            if (m_RasterState->cullEnabled)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);
            glFrontFace(m_RasterState->frontFace);
            glCullFace(m_RasterState->cullFace);
            glPolygonMode(GL_FRONT_AND_BACK, m_RasterState->polygonMode);
        }
    }

    DepthStencilState *
    OpenGLRenderDevice::CreateDepthStencilState(bool depthEnabled, bool depthWriteEnabled, float depthNear,
                                                float depthFar, Compare depthCompare,
                                                bool frontFaceStencilEnabled, Compare frontFaceStencilCompare,
                                                StencilAction frontFaceStencilFail, StencilAction frontFaceStencilPass,
                                                StencilAction frontFaceDepthFail, int frontFaceRef,
                                                unsigned int frontFaceReadMask, unsigned int frontFaceWriteMask,
                                                bool backFaceStencilEnabled,
                                                Compare backFaceStencilCompare, StencilAction backFaceStencilFail,
                                                StencilAction backFaceStencilPass, StencilAction backFaceDepthFail,
                                                int backFaceRef, unsigned int backFaceReadMask,
                                                unsigned int backFaceWriteMask) {
        m_depthStates.push_back(
                new OpenGLDepthStencilState(depthEnabled, depthWriteEnabled, depthNear, depthFar, depthCompare,
                                            frontFaceStencilEnabled, frontFaceStencilCompare,
                                            frontFaceStencilFail, frontFaceStencilPass, frontFaceDepthFail,
                                            frontFaceRef, frontFaceReadMask, frontFaceWriteMask, backFaceStencilEnabled,
                                            backFaceStencilCompare, backFaceStencilFail, backFaceStencilPass,
                                            backFaceDepthFail, backFaceRef, backFaceReadMask, backFaceWriteMask));
        return m_depthStates.back();
    }

    void OpenGLRenderDevice::DestroyDepthStencilState(DepthStencilState *depthStencilState) {
        if (depthStencilState) {
            m_depthStates.erase(
                    std::remove_if(m_depthStates.begin(), m_depthStates.end(),
                                   [depthStencilState](DepthStencilState *dState) {
                                       return *dState == *depthStencilState;
                                   }),
                    m_depthStates.end()
            );
            Utilities::Safe_Delete(depthStencilState);
        }
    }

    void OpenGLRenderDevice::SetDepthStencilState(DepthStencilState *depthStencilState) {
        DepthStencilState *oldDepthStencilState = m_DepthStencilState;

        if (depthStencilState)
            m_DepthStencilState = dynamic_cast<OpenGLDepthStencilState *>(depthStencilState);
        else
            m_DepthStencilState = m_DefaultDepthStencilState;

        if (m_DepthStencilState != oldDepthStencilState) {
            if (m_DepthStencilState->depthEnabled)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);
            glDepthFunc(m_DepthStencilState->depthFunc);
            glDepthMask(m_DepthStencilState->depthWriteEnabled ? GL_TRUE : GL_FALSE);
            glDepthRange(m_DepthStencilState->depthNear, m_DepthStencilState->depthFar);

            if (m_DepthStencilState->frontFaceStencilEnabled || m_DepthStencilState->backFaceStencilEnabled)
                glEnable(GL_STENCIL_TEST);
            else
                glDisable(GL_STENCIL_TEST);

            // front face
            glStencilFuncSeparate(GL_FRONT, m_DepthStencilState->frontStencilFunc, m_DepthStencilState->frontFaceRef,
                                  m_DepthStencilState->frontFaceReadMask);
            glStencilMaskSeparate(GL_FRONT, m_DepthStencilState->frontFaceWriteMask);
            glStencilOpSeparate(GL_FRONT, m_DepthStencilState->frontFaceStencilFail,
                                m_DepthStencilState->frontFaceDepthFail, m_DepthStencilState->frontFaceStencilPass);

            // back face
            glStencilFuncSeparate(GL_BACK, m_DepthStencilState->backStencilFunc, m_DepthStencilState->backFaceRef,
                                  m_DepthStencilState->backFaceReadMask);
            glStencilMaskSeparate(GL_BACK, m_DepthStencilState->backFaceWriteMask);
            glStencilOpSeparate(GL_BACK, m_DepthStencilState->backFaceStencilFail,
                                m_DepthStencilState->backFaceDepthFail, m_DepthStencilState->backFaceStencilPass);
        }
    }

    void OpenGLRenderDevice::Clear(float red, float green, float blue, float alpha, float depth, int stencil) {
        glClearColor(red, green, blue, alpha);
        glClearDepth(depth);
        glClearStencil(stencil);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void OpenGLRenderDevice::DrawPoints(long long offset, int count) {
        glDrawArrays(GL_POINTS, offset, count);
    }

    void OpenGLRenderDevice::DrawTriangles(int offset, int count) {
        glDrawArrays(GL_TRIANGLES, offset, count);
    }

    void OpenGLRenderDevice::DrawLineStrip(long long offset, int count) {
        glDrawArrays(GL_LINE_STRIP, offset, count);
    }

    void OpenGLRenderDevice::DrawTrianglesIndexed32(long long offset, int count) {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<const void *>(offset));
    }

    void OpenGLRenderDevice::BindDefaultPipeline() { SetPipeline(m_defaultPipeline); }

    void OpenGLRenderDevice::SetPointSize(float pSize) {
        glPointSize(pSize);
    }

    Pipeline *OpenGLRenderDevice::GetDefaultPipeline() { return dynamic_cast<Pipeline *>(m_defaultPipeline); }

    GLenum glCheckError_(const char *file, int line) {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR) {
            std::string error;
            switch (errorCode) {
                case GL_INVALID_ENUM: {
                    error = "INVALID_ENUM";
                    break;
                }
                case GL_INVALID_VALUE: {
                    error = "INVALID_VALUE";
                    break;
                }
                case GL_INVALID_OPERATION: {
                    error = "INVALID_OPERATION";
                    break;
                }
                    //case GL_STACK_OVERFLOW:                { error = "STACK_OVERFLOW"; break; }
                    //case GL_STACK_UNDERFLOW:               { error = "STACK_UNDERFLOW"; break; }
                case GL_OUT_OF_MEMORY: {
                    error = "OUT_OF_MEMORY";
                    break;
                }
                case GL_INVALID_FRAMEBUFFER_OPERATION: {
                    error = "INVALID_FRAMEBUFFER_OPERATION";
                    break;
                }
            }
            std::cout << error << " | " << file << " (" << line << ")" << std::endl;
        }
        return errorCode;
    }

    void OpenGLRenderDevice::DrawModel(Model & aModel, glm::mat4 & arcball, glm::mat4 & view, glm::mat4 & projection)
    {
        Pipeline * pipeline = aModel.GetPipeline();
        //Set MVP uniforms
        PipelineParam *uModel, *uView, *uProjection, *uNormalMatrix, *uArcball;
        uModel = pipeline->GetParam("uModel");
        uView = pipeline->GetParam("uView");
        uProjection = pipeline->GetParam("uProjection");
        uNormalMatrix = pipeline->GetParam("uNormalMatrix");
        uArcball = pipeline->GetParam("uArcball");

//        if (uModel)
//            uModel->SetAsMat4(glm::value_ptr(aModel.));
        if (uView)
            uView->SetAsMat4(glm::value_ptr(view));
        if (uProjection)
            uProjection->SetAsMat4(glm::value_ptr(projection));
        if (uNormalMatrix)
        {
            //Create the normal matrix
            glm::mat4 modelMat = aModel.GetModelMatrix();
            glm::mat4 normalMat = glm::mat3(glm::transpose(glm::inverse(arcball * modelMat)));
            uNormalMatrix->SetAsMat3(glm::value_ptr(normalMat));
        }
        if(uArcball)
            uArcball->SetAsMat4(glm::value_ptr(arcball));

        SetPipeline(pipeline);
        //For each of the model's meshes, draw
        for (size_t i = 0; i < aModel.NumMeshes(); i++)
        {
            Mesh * aMesh = aModel.GetMesh(i);
            //Bind each of the mesh's textures.
            for (size_t j = 0; j < aMesh->NumTextures(); j++)
                SetTexture2D(j, aMesh->GetTexture(j));

            //Bind the mesh's VAO and buffers
            SetVertexArray(aMesh->GetVertexArray());
            SetIndexBuffer(aMesh->GetIndexBuffer());

            //Draw!
            DrawTrianglesIndexed32(0, aMesh->NumIndices());
        }
    }
}