#include "RenderDevice.hpp"
#include "OpenGLRenderDevice.hpp"

namespace starforge
{
	RenderDevice *CreateRenderDevice()
	{
		// Eventually, we will parse a config, command-line argument, or global setting, in order to retrieve which RenderDevice should be instantiated.
		// Right now, we only have OpenGL.
		return new OpenGLRenderDevice;
	}

	void DestroyRenderDevice(RenderDevice *renderDevice)
	{
		delete renderDevice;
	}
}