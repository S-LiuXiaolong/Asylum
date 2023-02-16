#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>

#include "application.h"
#include "gl4ext.h"
#include "basiccamera.h"

#define TITLE				"STLReader"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// sample variables
Application* app = nullptr;

// OpenGLMesh* stlmesh = nullptr;
OpenGLMesh* mesh = nullptr;
OpenGLMesh* skymesh = nullptr;

OpenGLEffect* skyeffect = nullptr;
OpenGLEffect* blinnphong = nullptr;

OpenGLScreenQuad* screenquad = nullptr;

GLuint environment = 0;

BasicCamera camera;
BasicCamera debugcamera;
BasicCamera light;

bool use_debug = false;

bool InitScene()
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	glClearColor(0.0f, 0.125f, 0.3f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	if (!GLCreateMeshFromQM("../../../Asset/Mesh/QM/beanbag2.qm", &mesh)) {
		MYERROR("Could not load mesh");
		return false;
	}

	if (!GLCreateMeshFromQM("../../../Asset/Mesh/QM/sky.qm", &skymesh)) {
		MYERROR("Could not load sky");
		return false;
	}

// 	if (!GLCreateMeshFromSTL("../../../Asset/Mesh/STL/t13_simple.stl", stlmesh))
// 	{
// 		MYERROR("Could not load STL file");
// 		return false;
// 	}

	// load textures
	if (!GLCreateCubeTextureFromDDS("../../../Asset/Textures/ocean_env.dds", true, &environment))
		return false;

	// load shaders
	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/sky.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/sky.frag", &skyeffect)) {
		MYERROR("Could not load 'sky' shader");
		return false;
	}

	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/blinnphong.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/blinnphong.frag", &blinnphong)) {
		MYERROR("Could not load 'blinnphong' effect");
		return false;
	}

	Math::Matrix identity(1, 1, 1, 1);

	skyeffect->SetInt("sampler0", 0);

	screenquad = new OpenGLScreenQuad();

	// setup camera
	camera.SetAspect((float)screenwidth / screenheight);
	camera.SetFov(Math::HALF_PI);
	camera.SetClipPlanes(0.1f, 30.0f);
	camera.SetDistance(1.7f);
	camera.SetPosition(0, 0.5f, 0);
	camera.SetOrientation(Math::DegreesToRadians(135), 0.45f, 0);
	camera.SetPitchLimits(0.3f, Math::HALF_PI);

	return true;
}

void UninitScene()
{
//	delete stlmesh;
	delete mesh;
	delete skymesh;

	delete skyeffect;
	delete blinnphong;
	
	delete screenquad;

	GL_SAFE_DELETE_TEXTURE(environment);

	OpenGLContentManager().Release();
}

void Update(float delta)
{
	camera.Update(delta);
}

void Render(float alpha, float elapsedtime)
{
	Math::Matrix world, view, proj;
	Math::Matrix viewproj;
	Math::Vector3 eye;
	Math::Vector3 lightpos = { 6, 3, -10 };
	Math::Color color = { 1, 1, 1, 1 };

	camera.Animate(alpha);

	camera.GetViewMatrix(view);
	camera.GetProjectionMatrix(proj);
	camera.GetEyePosition(eye);

	Math::MatrixMultiply(viewproj, view, proj);
	Math::MatrixIdentity(world);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// sky
	Math::MatrixScaling(world, 20, 20, 20);

	world._41 = eye.x;
	world._42 = eye.y;
	world._43 = eye.z;

	skyeffect->SetVector("eyePos", eye);
	skyeffect->SetMatrix("matWorld", world);
	skyeffect->SetMatrix("matViewProj", viewproj);

	skyeffect->Begin();
	{
		GLSetTexture(GL_TEXTURE0, GL_TEXTURE_CUBE_MAP, environment);
		skymesh->Draw();
	}
	skyeffect->End();

	// object
	Math::MatrixIdentity(world);

	blinnphong->SetMatrix("matWorld", world);
	blinnphong->SetMatrix("matViewProj", viewproj);
	blinnphong->SetVector("lightPos", lightpos);
	blinnphong->SetVector("eyePos", eye);
	blinnphong->SetVector("matColor", &color.r);

	blinnphong->Begin();
	{
		blinnphong->CommitChanges();

		mesh->Draw();
	}
	blinnphong->End();

	// check errors
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
		std::cout << "Error\n";

	app->Present();
}

void KeyUp(KeyCode key)
{
	switch (key)
	{
	case KeyCodeR: {
		use_debug = !use_debug;
	} break;

	default:
		break;
	}
		

}

void MouseMove(int32_t x, int32_t y, int16_t dx, int16_t dy)
{
	uint8_t state = app->GetMouseButtonState();

	if (state & MouseButtonLeft) {
		camera.OrbitRight(Math::DegreesToRadians(dx));
		camera.OrbitUp(Math::DegreesToRadians(dy));
	}
}

int main(int argc, char* argv[])
{
	app = Application::Create(1360, 768);
	app->SetTitle(TITLE);

	if (!app->InitializeDriverInterface(GraphicsAPIOpenGL)) {
		delete app;
		return 1;
	}

	app->InitSceneCallback = InitScene;
	app->UninitSceneCallback = UninitScene;
	app->UpdateCallback = Update;
	app->RenderCallback = Render;
	app->KeyUpCallback = KeyUp;
	app->MouseMoveCallback = MouseMove;

	app->Run();
	delete app;

	return 0;
}