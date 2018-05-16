#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ProgMesh.hpp"
#include "ProgModel.hpp"
#include "ProgMesh.hpp"

#include "Platform.hpp"
#include "RenderDevice.hpp"

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
ProgModelRef aModel;

int main(int argc, char *argv[]) {
    if(argc <= 1) {
        std::cerr << "ERROR: Please provide a model file as input" << std::endl;
        return -1;
    }

    platform::InitPlatform();
    platform::PLATFORM_WINDOW_REF window = platform::CreatePlatformWindow(1024, 768, "Progressive Meshes");
    if (!window) {
        platform::TerminatePlatform();
        return -1;
    }
    glfwSetKeyCallback((GLFWwindow *)window, keyboard_callback);

    starforge::RenderDevice *renderDevice = starforge::CreateRenderDevice();

    // Load the shaders and create the pipeline.
    std::ifstream vShaderFile("data/shaders/standard.vert");
    if(!vShaderFile.good()) {
        std::cerr << "ERROR: Unable to find shader standard.vert" << std::endl;
        return -1;
    }
    std::string vShaderSource(static_cast<std::stringstream const&>(std::stringstream() << vShaderFile.rdbuf()).str());
    starforge::VertexShader * vertexShader = renderDevice->CreateVertexShader(vShaderSource.c_str());

    std::ifstream fShaderFile("data/shaders/standard.frag");
    if(!fShaderFile.good()) {
        std::cerr << "ERROR: Unable to find shader standard.frag" << std::endl;
        return -1;
    }
    std::string fShaderSource(static_cast<std::stringstream const&>(std::stringstream() << fShaderFile.rdbuf()).str());
    starforge::PixelShader * fragShader = renderDevice->CreatePixelShader(fShaderSource.c_str());

    starforge::Pipeline * pipeline = renderDevice->CreatePipeline(vertexShader, fragShader);

    // Once pipeline has been created, no need for shaders
    renderDevice->DestroyVertexShader(vertexShader);
    renderDevice->DestroyPixelShader(fragShader);

    // Get shader parameters we will set it every frame
    starforge::PipelineParam *uModelParam = pipeline->GetParam("uModel");
    starforge::PipelineParam *uViewParam = pipeline->GetParam("uView");
    starforge::PipelineParam *uProjectionParam = pipeline->GetParam("uProjection");
    starforge::PipelineParam * uArcballParam = pipeline->GetParam("uArcball");
    starforge::PipelineParam * uNormalMatParam = pipeline->GetParam("uNormalMatrix");

    aModel = std::make_shared<ProgModel>(std::string(argv[1]));
    aModel->PrintInfo(std::cout);
    for(auto & aMesh: aModel->GetMeshes()) {
        aMesh->AllocateBuffers(*renderDevice);
    }

    renderDevice->SetPipeline(pipeline);

    // Main run loop
    while(platform::PollPlatformWindow(window)) {
        renderDevice->Clear(0.2f, 0.3f, 0.3f);
        renderDevice->SetPipeline(pipeline);
        glm::mat4 arcball, view, projection;
        platform::GetPlatformViewport(arcball, view, projection);

        uArcballParam->SetAsMat4(glm::value_ptr(arcball));
        uViewParam->SetAsMat4(glm::value_ptr(view));
        uProjectionParam->SetAsMat4(glm::value_ptr(projection));

        for(ProgMeshRef aMesh: aModel->GetMeshes()) {
            auto & modelMat = aMesh->GetModelMatrix();
            uModelParam->SetAsMat4(glm::value_ptr(modelMat));

            glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(modelMat * arcball)));
            uNormalMatParam->SetAsMat3(glm::value_ptr(normMat));

            aMesh->Draw(*renderDevice);
        }

        platform::PresentPlatformWindow(window);
    }

    renderDevice->DestroyPipeline(pipeline);
    aModel.reset();

    starforge::DestroyRenderDevice(renderDevice);
    platform::TerminatePlatform();
    return 0;
}

static void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS) {

		for (auto aMesh : aModel->GetMeshes()) {

			aMesh->CollapseLeastError();

		}

	}

}