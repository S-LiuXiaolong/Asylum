
#include "application.h"

#ifdef _WIN32
#	include "win32application.h"
#endif

static void ReadResolutionFile(uint32_t& width, uint32_t& height)
{
	FILE* infile = nullptr;
	
	fopen_s(&infile, "res.conf", "rb");

	if (infile != nullptr) {
		fscanf_s(infile, "%lu %lu\n", &width, &height);
		fclose(infile);

		if (width < 640)
			width = 640;

		if (height < 480)
			height = 480;
	} else {
		fopen_s(&infile, "res.conf", "wb");

		if (infile != nullptr) {
			fprintf(infile, "%ud %ud\n", width, height);
			fclose(infile);
		}
	}
}

Application::~Application()
{
}

Application* Application::Create(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0) {
		// use config file values
		width = 1920;
		height = 1080;

		ReadResolutionFile(width, height);
	}

#ifdef _WIN32
	return new Win32Application(width, height);
#elif defined(__APPLE__)
	return new macOSApplication(width, height);
#else
	return nullptr;
#endif
}
