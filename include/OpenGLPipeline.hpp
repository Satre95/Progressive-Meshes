#pragma once

#include <glad/glad.h>
#include <map>
#include <vector>
#include "RenderDevice.hpp"

namespace starforge {

    class OpenGLVertexShader : public VertexShader {
    public:

        OpenGLVertexShader(const char *code) {
            // ------------------------------------
            // vertex shader
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &code, NULL);
            glCompileShader(vertexShader);

            // check for shader compile errors
            int success;
            char infoLog[512];
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }

        ~OpenGLVertexShader() override {
            glDeleteShader(vertexShader);
        }

        int vertexShader = 0;
    };

    class OpenGLPixelShader : public PixelShader {
    public:

        OpenGLPixelShader(const char *code) {
            // fragment shader
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &code, NULL);
            glCompileShader(fragmentShader);

            // check for shader compile errors
            int success;
            char infoLog[512];
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
            }
        }

        ~OpenGLPixelShader() override {
            glDeleteShader(fragmentShader);
        }

        int fragmentShader = 0;
    };

    class OpenGLPipelineParam;

    class OpenGLPipeline : public Pipeline {
    public:

        OpenGLPipeline(OpenGLVertexShader *vertexShader, OpenGLPixelShader *pixelShader) {
            // link shaders
            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertexShader->vertexShader);
            glAttachShader(shaderProgram, pixelShader->fragmentShader);
            glLinkProgram(shaderProgram);
            //Detach the shaders, not needed anymore
            glDetachShader(shaderProgram, vertexShader->vertexShader);
            glDetachShader(shaderProgram, pixelShader->fragmentShader);

            // check for linking errors
            int success;
            char infoLog[512];
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
        }

        ~OpenGLPipeline() override {
            glDeleteProgram(shaderProgram);
        }

        PipelineParam *GetParam(const char *name) override;

        bool operator==(const Pipeline &other) const override {
            return shaderProgram == static_cast<const OpenGLPipeline &>(other).shaderProgram;
        }

        int shaderProgram = 0;

        std::map<std::string, OpenGLPipelineParam *> paramsByName;
    };

    class OpenGLPipelineParam : public PipelineParam {
    public:

        OpenGLPipelineParam(OpenGLPipeline *_pipeline, int _location) : pipeline(_pipeline), location(_location) {}

        void SetAsBool(bool value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform1i(location, value);
        }

        void SetAsInt(int value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform1i(location, value);
        }

        void SetAsFloat(float value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform1f(location, value);
        }

        void SetAsMat4(const float *value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniformMatrix4fv(location, 1, /*transpose=*/GL_FALSE, value);
        }

        void SetAsMat3(const float *value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniformMatrix3fv(location, 1, GL_FALSE, value);
        }

        void SetAsVec3(const float *value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform3fv(location, 1, value);
        }

        void SetAsVec4(const float *value) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform4fv(location, 1, value);
        }

        void SetAsIntArray(int count, const int *values) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform1iv(location, count, values);
        }

        void SetAsFloatArray(int count, const float *values) override {
            glUseProgram(pipeline->shaderProgram);
            glUniform1fv(location, count, values);
        }

        void SetAsMat4Array(int count, const float *values) override {
            glUseProgram(pipeline->shaderProgram);
            glUniformMatrix4fv(location, count, /*transpose=*/GL_FALSE, values);
        }

        OpenGLPipeline *pipeline;
        int location;
    };
}