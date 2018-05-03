#include <iostream>
#include <string>

#include "ProgMesh.hpp"
#include "ProgModel.hpp"
#include "ProgMesh.hpp"

#include "Platform.hpp"
#include "RenderDevice.hpp"


int main(int argc, char *argv[]) {
    if(argc <= 1) {
        std::cerr << "ERROR: Please provide a model file as input" << std::endl;
        return 1;
    }

    platform::InitPlatform();
    platform::PLATFORM_WINDOW_REF window = platform::CreatePlatformWindow(800, 800, "Cube");
    if (!window) {
        platform::TerminatePlatform();
        return -1;
    }

    starforge::RenderDevice *renderDevice = starforge::CreateRenderDevice();

    ProgModel * aModel = new ProgModel(std::string(argv[1]));
    aModel->PrintInfo(std::cout);

    delete aModel;

    starforge::DestroyRenderDevice(renderDevice);
    platform::TerminatePlatform();
    return 0;
}