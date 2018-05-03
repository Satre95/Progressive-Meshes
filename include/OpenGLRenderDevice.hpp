#pragma once
#include <vector>
#include <glad/glad.h>
#include <iostream>
#include "RenderDevice.hpp"
#include "DefaultShaders.hpp"
#include "OpenGLDepthRasterStates.hpp"
#include "OpenGLPipeline.hpp"

namespace starforge
{
	class OpenGLVertexDescription : public VertexDescription
	{
	public:

		struct OpenGLVertexElement
		{
			GLuint index;
			GLint size;
			GLenum type;
			GLboolean normalized;
			GLsizei stride;
			const GLvoid *pointer;
		};

		OpenGLVertexDescription(unsigned int _numVertexElements, const VertexElement *vertexElements) :
				numVertexElements(_numVertexElements), id(count++)
		{
			static GLenum toOpenGLType[] = { GL_BYTE, GL_SHORT, GL_INT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT,
											 GL_BYTE, GL_SHORT, GL_INT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE };
			static GLboolean toOpenGLNormalized[] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE,
													  GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE };

			openGLVertexElements = new OpenGLVertexElement[numVertexElements];
			for (unsigned int i = 0; i < numVertexElements; i++)
			{
				openGLVertexElements[i].index = vertexElements[i].index;
				openGLVertexElements[i].size = vertexElements[i].size;
				openGLVertexElements[i].type = toOpenGLType[vertexElements[i].type];
				openGLVertexElements[i].normalized = toOpenGLNormalized[vertexElements[i].type];
				openGLVertexElements[i].stride = vertexElements[i].stride;
				openGLVertexElements[i].pointer = (char *)nullptr + vertexElements[i].offset;
			}
		}

		~OpenGLVertexDescription() override
		{
			delete[] openGLVertexElements;
		}

		OpenGLVertexDescription(const OpenGLVertexDescription & other):
				id(other.id)
		{
			numVertexElements = other.numVertexElements;
			std::copy(other.openGLVertexElements, other.openGLVertexElements + numVertexElements, openGLVertexElements);
		}

		bool operator==(const VertexDescription & obj) const override {
			return id == (static_cast<const OpenGLVertexDescription &>(obj).id);
		};


		virtual unsigned int NumElements() override { return numVertexElements; }
		unsigned int numVertexElements = 0;
		OpenGLVertexElement *openGLVertexElements = nullptr;
	private:
		const size_t id;
		static size_t count;
	};

	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:

		OpenGLVertexBuffer(long long size, const void *data)
		{
			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); // always assuming dynamic, for now
		}

		~OpenGLVertexBuffer() override
		{
			glDeleteBuffers(1, &VBO);
		}

		bool operator ==(const VertexBuffer & obj) const override
		{
			return (VBO == static_cast<const OpenGLVertexBuffer &>(obj).VBO);
		}


		unsigned int VBO = 0;
	};

	class OpenGLVertexArray : public VertexArray
	{
	public:

		OpenGLVertexArray(unsigned int numVertexBuffers, VertexBuffer **vertexBuffers, VertexDescription **vertexDescriptions)
		{
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			for (unsigned int i = 0; i < numVertexBuffers; i++)
			{
				OpenGLVertexBuffer *vertexBuffer = reinterpret_cast<OpenGLVertexBuffer *>(vertexBuffers[i]);
				OpenGLVertexDescription *vertexDescription = reinterpret_cast<OpenGLVertexDescription *>(vertexDescriptions[i]);

				glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->VBO);

				for (unsigned int j = 0; j < vertexDescription->NumElements(); j++)
				{
					glEnableVertexAttribArray(vertexDescription->openGLVertexElements[j].index);
					glVertexAttribPointer(vertexDescription->openGLVertexElements[j].index, vertexDescription->openGLVertexElements[j].size, vertexDescription->openGLVertexElements[j].type,
										  vertexDescription->openGLVertexElements[j].normalized, vertexDescription->openGLVertexElements[j].stride, vertexDescription->openGLVertexElements[j].pointer);
				}
			}
		}

		bool operator ==(const VertexArray & obj) const override
		{
			return (VAO == static_cast<const OpenGLVertexArray &>(obj).VAO);
		}

		~OpenGLVertexArray() override
		{
			glDeleteVertexArrays(1, &VAO);
		}

		unsigned int VAO = 0;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:

		OpenGLIndexBuffer(long long size, const void *data)
		{
			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); // always assuming static, for now
		}

		~OpenGLIndexBuffer() override
		{
			glDeleteBuffers(1, &IBO);
		}

		bool operator ==(const IndexBuffer & obj) const override
		{
			return (IBO == static_cast<const OpenGLIndexBuffer &>(obj).IBO);
		}

		unsigned int IBO = 0;
	};


	class OpenGLRenderDevice : public RenderDevice
	{
	public:
		OpenGLRenderDevice();
		~OpenGLRenderDevice() override;

		VertexShader *CreateVertexShader(const char *code) override;

		void DestroyVertexShader(VertexShader *vertexShader) override;

		PixelShader *CreatePixelShader(const char *code) override;

		void DestroyPixelShader(PixelShader *pixelShader) override;

		Pipeline *CreatePipeline(VertexShader *vertexShader, PixelShader *pixelShader) override;

		void DestroyPipeline(Pipeline *pipeline) override;

		void SetPipeline(Pipeline *pipeline) override;

		VertexBuffer *CreateVertexBuffer(long long size, const void *data = nullptr) override;

		void DestroyVertexBuffer(VertexBuffer *vertexBuffer) override;

		VertexDescription *CreateVertexDescription(unsigned int numVertexElements, const VertexElement *vertexElements) override;

		void DestroyVertexDescription(VertexDescription *vertexDescription) override;

		VertexArray *CreateVertexArray(unsigned int numVertexBuffers, VertexBuffer **vertexBuffers, VertexDescription **vertexDescriptions) override;

		void DestroyVertexArray(VertexArray *vertexArray) override;

		void SetVertexArray(VertexArray *vertexArray) override;

		IndexBuffer *CreateIndexBuffer(long long size, const void *data = nullptr) override;

		void DestroyIndexBuffer(IndexBuffer *indexBuffer) override;

		void SetIndexBuffer(IndexBuffer *indexBuffer) override;

		Texture2D *CreateTexture2D(int width, int height, const void *data = nullptr) override;

		void DestroyTexture2D(Texture2D *texture2D) override;

		void SetTexture2D(unsigned int slot, Texture2D *texture2D) override;

		RasterState *CreateRasterState(bool cullEnabled = false, Winding frontFace = WINDING_CCW, Face cullFace = FACE_BACK, RasterMode rasterMode = RASTERMODE_FILL) override;

		void DestroyRasterState(RasterState *rasterState) override;

		void SetRasterState(RasterState *rasterState) override;

		DepthStencilState *CreateDepthStencilState(bool depthEnabled = true,
												   bool			depthWriteEnabled = true,
												   float			depthNear = 0,
												   float			depthFar = 1,
												   Compare			depthCompare = COMPARE_LESS,

												   bool			frontFaceStencilEnabled = false,
												   Compare			frontFaceStencilCompare = COMPARE_ALWAYS,
												   StencilAction	frontFaceStencilFail = STENCIL_KEEP,
												   StencilAction	frontFaceStencilPass = STENCIL_KEEP,
												   StencilAction	frontFaceDepthFail = STENCIL_KEEP,
												   int				frontFaceRef = 0,
												   unsigned int	frontFaceReadMask = 0xFFFFFFFF,
												   unsigned int	frontFaceWriteMask = 0xFFFFFFFF,

												   bool			backFaceStencilEnabled = false,
												   Compare			backFaceStencilCompare = COMPARE_ALWAYS,
												   StencilAction	backFaceStencilFail = STENCIL_KEEP,
												   StencilAction	backFaceStencilPass = STENCIL_KEEP,
												   StencilAction	backFaceDepthFail = STENCIL_KEEP,
												   int				backFaceRef = 0,
												   unsigned int	backFaceReadMask = 0xFFFFFFFF,
												   unsigned int	backFaceWriteMask = 0xFFFFFFFF) override;

		void DestroyDepthStencilState(DepthStencilState *depthStencilState) override;

		void SetDepthStencilState(DepthStencilState *depthStencilState) override;

		void Clear(float red = 0.0f, float green = 0.0f, float blue = 0.0f, float alpha = 1.0f, float depth = 1.0f, int stencil = 0) override;

		void DrawTriangles(int offset, int count) override;

		void DrawPoints(long long offset, int count) override;

		void DrawLineStrip(long long offset, int count) override;

		void DrawTrianglesIndexed32(long long offset, int count) override;

		Pipeline * GetDefaultPipeline() override;

		void BindDefaultPipeline() override;

		void SetPointSize(float pSize) override;

		void DrawModel(Model & aModel, glm::mat4 & arcball, glm::mat4 & view, glm::mat4 & projection) override;

	private:
		OpenGLRasterState *m_RasterState = nullptr;
		OpenGLRasterState *m_DefaultRasterState = nullptr;

		OpenGLDepthStencilState *m_DepthStencilState = nullptr;
		OpenGLDepthStencilState *m_DefaultDepthStencilState = nullptr;

		OpenGLPipeline * m_defaultPipeline = nullptr;

		// Tracks all the buffers that have been allocated.
		std::vector<OpenGLVertexArray *> m_VAOs;
		std::vector<OpenGLVertexBuffer *> m_VBOs;
		std::vector<OpenGLIndexBuffer *> m_IBOs;
		std::vector<OpenGLVertexDescription *> m_vDescriptions;
		std::vector<OpenGLRasterState *> m_rasterStates;
		std::vector<OpenGLDepthStencilState *> m_depthStates;
		std::vector<OpenGLPipeline *> m_pipelines;
	};

	GLenum glCheckError_(const char *file, int line);
#define CheckRenderError() glCheckError_(__FILE__, __LINE__)
}