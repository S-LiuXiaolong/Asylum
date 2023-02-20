#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>

#include "application.h"
#include "gl4ext.h"
#include "basiccamera.h"

#include "inpUtil.h"

#define TITLE				"INPReader"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// sample variables
Application* app = nullptr;

// OpenGLMesh* stlmesh = nullptr;
OpenGLMesh* mesh = nullptr;

OpenGLEffect* blinnphong = nullptr;

OpenGLScreenQuad* screenquad = nullptr;

GLuint environment = 0;

BasicCamera camera;
BasicCamera debugcamera;
BasicCamera light;

bool use_debug = false;

std::shared_ptr<Element> ele;

void GenerateMesh()
{
	std::shared_ptr<Node> node1(new Node());
	std::shared_ptr<Node> node2(new Node());
	std::shared_ptr<Node> node3(new Node());
	std::shared_ptr<Node> node4(new Node());
	std::shared_ptr<Node> node5(new Node());
	node1->SetCoordinate(1.0f, 1.0f, 0.0f);
	node2->SetCoordinate(1.0f, 0.0f, 0.0f);
	node3->SetCoordinate(1.5f, 0.5f, 0.707f);
	node4->SetCoordinate(0.5f, 0.5f, 0.707f);
	node5->SetCoordinate(1.0f, 2.0f, 2.0f);


	if (*node2.get() == *node3.get()) {
		std::cout << "node2 = node3 pass" << std::endl;
	}
	else {
		std::cout << "node2 = node3 not pass" << std::endl;
	}

	std::shared_ptr<Face> face1(new Face());
	std::shared_ptr<Face> face2(new Face());
	std::shared_ptr<Face> face3(new Face());
	std::shared_ptr<Face> face4(new Face());
	std::shared_ptr<Face> face5(new Face());
	face1->SetNodes(node1, node2, node3);
	face2->SetNodes(node1, node3, node2);
	face3->SetNodes(node1, node2, node5);
	face4->SetNodes(node2, node3, node4);
	face5->SetNodes(node2, node3, node5);


	if (*face1.get() == *face2.get()) {
		std::cout << "face1 = face2 pass" << std::endl;
	}
	else {
		std::cout << "face1 = face2 not pass" << std::endl;
	}

	std::shared_ptr<Element> ele1(new Element());
	std::shared_ptr<Element> ele2(new Element());
	ele1->SetNodes(node1, node2, node3, node4);
	ele2->SetNodes(node1, node2, node3, node5);

	if (*ele1.get() == *ele2.get()) {
		std::cout << "ele1 = ele2 pass" << std::endl;
	}
	else {
		std::cout << "ele1 = ele2 not pass" << std::endl;
	}

	ele = ele1;

}

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

// 	if (!GLCreateMeshFromQM("../../../Asset/Mesh/QM/beanbag2.qm", &mesh)) {
// 		MYERROR("Could not load mesh");
// 		return false;
// 	}
	GenerateMesh();

	// create mesh
	OpenGLVertexElement decl[] = {
		{ 0, 0, GLDECLTYPE_FLOAT3, GLDECLUSAGE_POSITION, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	// TODO: how to get the index count and index buffer?
	if (!GLCreateMesh(4, 4, GLMESH_32BIT, decl, &mesh))
		return false;

	OpenGLAttributeRange* subsettable = nullptr;
	Math::Vector3* vdata = nullptr;
	uint32_t* idata = nullptr;
	GLuint numsubsets = 0;

	// TODO: remember to subtract with 1 when putting index into indexbuffer
	// (index of INP node start with 1 and OpenGL indexbuffer start with 0)
	mesh->LockVertexBuffer(0, 0, GLLOCK_DISCARD, (void**)&vdata);
	mesh->LockIndexBuffer(0, 0, GLLOCK_DISCARD, (void**)&idata);
	{
		// vertex data
// 		for (int z = 0; z <= MESH_SIZE; ++z) {
// 			for (int x = 0; x <= MESH_SIZE; ++x) {
// 				int index = z * (MESH_SIZE + 1) + x;
// 
// 				vdata[index].x = (float)x;
// 				vdata[index].y = (float)z;
// 				vdata[index].z = 0.0f;
// 			}
// 		}
		for (int i = 0; i < 4; i++) {
			vdata[i].x = 
		}

		// index data
		GenerateLODLevels(&subsettable, &numsubsets, idata);
	}
	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();

	mesh->SetAttributeTable(subsettable, numsubsets);
	delete[] subsettable;




// 	if (!GLCreateMeshFromSTL("../../../Asset/Mesh/STL/t13_simple.stl", stlmesh))
// 	{
// 		MYERROR("Could not load STL file");
// 		return false;
// 	}

	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/blinnphong.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/blinnphong.frag", &blinnphong)) {
		MYERROR("Could not load 'blinnphong' effect");
		return false;
	}

	Math::Matrix identity(1, 1, 1, 1);

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