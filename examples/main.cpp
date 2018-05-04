#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "ProgMesh.hpp"
#include "ProgModel.hpp"
#include "ProgMesh.hpp"

#include "Platform.hpp"
#include "RenderDevice.hpp"


int main(int argc, char *argv[]) {
    if(argc <= 1) {
        std::cerr << "ERROR: Please provide a model file as input" << std::endl;
        return -1;
    }

    platform::InitPlatform();
    platform::PLATFORM_WINDOW_REF window = platform::CreatePlatformWindow(800, 800, "Cube");
    if (!window) {
        platform::TerminatePlatform();
        return -1;
    }

    starforge::RenderDevice *renderDevice = starforge::CreateRenderDevice();

    // Load the shaders and create the pipeline.
    std::ifstream vShaderFile("../data/shaders/standard.vert");
    if(!vShaderFile.good()) {
        std::cerr << "ERROR: Unable to find shader standard.vert" << std::endl;
        return -1;
    }
    std::string vShaderSource(static_cast<std::stringstream const&>(std::stringstream() << vShaderFile.rdbuf()).str());
    starforge::VertexShader * vertexShader = renderDevice->CreateVertexShader(vShaderSource.c_str());

    std::ifstream fShaderFile("../data/shaders/standard.frag");
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


    ProgModel * aModel = new ProgModel(std::string(argv[1]));
    aModel->PrintInfo(std::cout);

    // Main run loop
    while(platform::PollPlatformWindow(window)) {

        platform::PresentPlatformWindow(window);
    }

    delete aModel;

    renderDevice->DestroyPipeline(pipeline);

    starforge::DestroyRenderDevice(renderDevice);
    platform::TerminatePlatform();
    return 0;
}