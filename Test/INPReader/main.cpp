#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>

#include "application.h"
#include "gl4ext.h"
#include "geometryutils.h"
#include "basiccamera.h"

#include "inpUtil.h"

#define TITLE				"INPReader"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// sample variables
Application* app = nullptr;

// OpenGLMesh* stlmesh = nullptr;
OpenGLMesh* mesh = nullptr;
INPMesh inpMesh;

OpenGLEffect* blinnphong = nullptr;

OpenGLScreenQuad* screenquad = nullptr;

GLuint environment = 0;

BasicCamera camera;
BasicCamera debugcamera;
BasicCamera light;

bool use_debug = false;

typedef GeometryUtils::CommonVertex CommonVertex;

bool InitScene()
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

// 	if (!GLCreateMeshFromQM("../../../Asset/Mesh/QM/beanbag2.qm", &mesh)) {
// 		MYERROR("Could not load mesh");
// 		return false;
// 	}

	// create mesh
	OpenGLVertexElement decl[] = {
		{ 0, 0, GLDECLTYPE_FLOAT3, GLDECLUSAGE_POSITION, 0 },
		{ 0, 12, GLDECLTYPE_FLOAT3, GLDECLUSAGE_NORMAL, 0 },
		{ 0, 24, GLDECLTYPE_FLOAT2, GLDECLUSAGE_TEXCOORD, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	// TODO: how to get the index count and index buffer?
	inpMesh.LoadFromFile("../../../Asset/Mesh/INP/boat.inp");
	auto& nodes = inpMesh.GetNodes();
	auto& elements = inpMesh.GetElements();
	auto& indices = inpMesh.GetIndexBuffer();

	if (!GLCreateMesh(indices.size(), indices.size(), GLMESH_32BIT, decl, &mesh))
		return false;

	OpenGLAttributeRange table[] = {
		{ GLPT_TRIANGLELIST, 0, 0, indices.size(), 0, nodes.size(), true }
	};
	CommonVertex* vdata = nullptr;
	int* idata = new int[indices.size()];
	GLuint numsubsets = 1;

	// TODO: remember to subtract with 1 when putting index into indexbuffer
	// (index of INP node start with 1 and OpenGL indexbuffer start with 0)
	mesh->LockVertexBuffer(0, 0, GLLOCK_DISCARD, (void**)&vdata);
	mesh->LockIndexBuffer(0, 0, GLLOCK_DISCARD, (void**)&idata);
	{
		// vertex data
		{
// 			for (int index = 0; index < indices.size() / 3; index++)
// 			{
// 				std::vector<Element> eles{ *elements[index * 3].get(), *elements[index * 3 + 1].get(), *elements[index * 3 + 2].get() };
// 
// 			}
			int vertexCount = 0;
			for (auto& ele : elements)
			{
				auto pFaces = ele->GetFaces();
				std::vector<Face> faces{ *pFaces[0].get(), *pFaces[1].get(), *pFaces[2].get(), *pFaces[3].get() };
				for (auto& face : faces)
				{
					auto pNodes = face.GetNodes();
					std::vector<Node> nodes{ *pNodes[0].get(), *pNodes[1].get(), *pNodes[2].get() };
					for (auto& node : nodes)
					{
						vdata[vertexCount].x = node.GetCoordinate()[0];
						vdata[vertexCount].y = node.GetCoordinate()[1];
						vdata[vertexCount].z = node.GetCoordinate()[2];

						vdata[vertexCount].nx = face.GetNormal().x;
						vdata[vertexCount].ny = face.GetNormal().y;
						vdata[vertexCount].nz = face.GetNormal().z;

						++vertexCount;
					}
				}
			}
		}

		// index data
		{
			for (int index = 0; index < indices.size(); index++)
			{
// 				auto temp = indices[index];
// 				temp = idata[index];
// 				idata[index] = indices[index];
				idata[index] = index;
			}
		}
	}
	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();

	mesh->SetAttributeTable(table, numsubsets);

	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/blinnphong.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/blinnphong.frag", &blinnphong)) {
		MYERROR("Could not load 'blinnphong' effect");
		return false;
	}
	else {
		std::cout << "Shader read finish!" << std::endl;
	}

	Math::Matrix identity(0, 1, 1, 1);

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
	Math::Vector3 lightpos = { -1, 1, -1 };
	Math::Color color = { 0, 1, 1, 1 };

	camera.Animate(alpha);

	camera.GetViewMatrix(view);
	camera.GetProjectionMatrix(proj);
	camera.GetEyePosition(eye);

	Math::MatrixMultiply(viewproj, view, proj);
	Math::MatrixIdentity(world);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// object
	Math::MatrixIdentity(world);

// 	world._41 = eye.x;
// 	world._42 = eye.y;
// 	world._43 = eye.z;

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