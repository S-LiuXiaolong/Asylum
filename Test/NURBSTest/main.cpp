
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>
#include <sstream>

#include "application.h"
#include "gl4ext.h"
#include "basiccamera.h"

#define MAX_NUM_SEGMENTS	100

// helper macros
#define TITLE				"NURBS tessellation"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// control constants
const GLuint NumControlVertices				= 7;
const GLuint NumControlIndices				= (NumControlVertices - 1) * 2;
const GLuint MaxSplineVertices				= MAX_NUM_SEGMENTS + 1;
const GLuint MaxSplineIndices				= (MaxSplineVertices - 1) * 2;
const GLuint MaxSurfaceVertices				= MaxSplineVertices * MaxSplineVertices;
const GLuint MaxSurfaceIndices				= (MaxSplineVertices - 1) * (MaxSplineVertices - 1) * 6;

// sample structures
struct CurveData
{
	int degree;	// max 3
	Math::Vector4 controlpoints[NumControlVertices];
	float weights[NumControlVertices + 4];
	float knots[11];
};

CurveData curves[] =
{
	// degree 3, "good"
	{
		3,
		{ { 1, 1, 0, 1 }, { 1, 5, 0, 1 }, { 3, 6, 0, 1 }, { 6, 3, 0, 1 }, { 9, 4, 0, 1 }, { 9, 9, 0, 1 }, { 5, 6, 0, 1 } },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 0.4f, 0.4f, 0.4f, 1, 1, 1, 1 }
	},

	// degree 3, "bad"
	{
		3,
		{ { 1, 1, 0, 1 }, { 1, 5, 0, 1 }, { 3, 6, 0, 1 }, { 6, 3, 0, 1 }, { 9, 4, 0, 1 }, { 9, 9, 0, 1 }, { 5, 6, 0, 1 } },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1 }
	},

	// degree 2, "good"
	{
		2,
		{ { 1, 1, 0, 1 }, { 1, 5, 0, 1 }, { 3, 6, 0, 1 }, { 6, 3, 0, 1 }, { 9, 4, 0, 1 }, { 9, 9, 0, 1 }, { 5, 6, 0, 1 } },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0.2f, 0.4f, 0.6f, 0.8f, 1, 1, 1 }
	},

	// degree 1
	{
		1,
		{ { 1, 1, 0, 1 }, { 1, 5, 0, 1 }, { 3, 6, 0, 1 }, { 6, 3, 0, 1 }, { 9, 4, 0, 1 }, { 9, 9, 0, 1 }, { 5, 6, 0, 1 } },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0.15f, 0.3f, 0.45f, 0.6f, 0.75f, 1, 1 }
	},

	// circle
	{
		2,
		{ { 5, 1, 0, 1 }, { 1, 1, 0, 1 }, { 3, 4.46f, 0, 1 }, { 5, 7.92f, 0, 1 }, { 7, 4.46f, 0, 1 }, { 9, 1, 0, 1 }, { 5, 1, 0, 1 }, },
		{ 1, 0.5f, 1, 0.5f, 1, 0.5f, 1 },
		{ 0, 0, 0, 0.33f, 0.33f, 0.67f, 0.67f, 1, 1, 1 }
	},
};

// sample variables
Application*		app					= nullptr;

OpenGLMesh*			surface				= nullptr;
OpenGLEffect*		tessellatesurfacemy	= nullptr;
OpenGLEffect*		rendersurface		= nullptr;
OpenGLScreenQuad*	screenquad			= nullptr;

BasicCamera			camera;
float				selectiondx			= 0;
float				selectiondy			= 0;
// int					numsegments			= MAX_NUM_SEGMENTS / 2;
int					numSegBetweenKnotsU = 10;
int					numSegBetweenKnotsV = 10;
int					currentcurve		= 4;
bool				wireframe			= false;

void Tessellate();

bool InitScene()
{
	if (GLExtensions::GLVersion < GLExtensions::GL_4_3) {
		MYERROR("This sample requires at least OpenGL 4.3");
		return false;
	}

	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	glClearColor(0.0f, 0.125f, 0.3f, 1.0f);
	glClearDepth(1.0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	OpenGLVertexElement decl2[] = {
		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	// create surface
	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
		MYERROR("Could not create surface");
		return false;
	}

	// load shaders
	// TODO: Add a more clear way
	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/rendersurface.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/rendersurface.frag", &rendersurface)) {
		MYERROR("Could not load surface renderer shader");
		return false;
	}

	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
		MYERROR("Could not load compute shader");
		return false;
	}

	screenquad = new OpenGLScreenQuad();

	// tessellate for the first time
	Tessellate();

	// setup camera
	camera.SetAspect((float)screenwidth / screenheight);
	camera.SetFov(Math::DegreesToRadians(60));
	camera.SetClipPlanes(0.1f, 50.0f);
	camera.SetDistance(20);
	camera.SetOrientation(Math::HALF_PI, 0.5f, 0);
	camera.SetPosition(5, 4, 5);

	return true;
}

void UninitScene()
{
	delete tessellatesurfacemy;
	delete rendersurface;
	delete surface;
	delete screenquad;

	OpenGLContentManager().Release();
}

void Tessellate()
{
	CurveData& current = curves[currentcurve];

	// update surface cvs
	Math::Vector4* surfacecvs = new Math::Vector4[NumControlVertices * NumControlVertices];
	GLuint index;

	for (GLuint i = 0; i < NumControlVertices; ++i) {
		for (GLuint j = 0; j < NumControlVertices; ++j) {
			index = i * NumControlVertices + j;

			surfacecvs[index][0] = current.controlpoints[i][0];
			surfacecvs[index][2] = current.controlpoints[j][0];
			surfacecvs[index][1] = (current.controlpoints[i][1] + current.controlpoints[j][1]) * 0.5f;
		}
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());

// 	if (current.degree > 1) {
// 		tessellatesurfacemy->SetInt("numVerticesU", numsegments + 1);
// 		tessellatesurfacemy->SetInt("numVerticesV", numsegments + 1);
// 	} else {
// 		tessellatesurfacemy->SetInt("numVerticesU", NumControlVertices);
// 		tessellatesurfacemy->SetInt("numVerticesV", NumControlVertices);
// 	}

	if (current.degree > 1) {
		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
	} else {
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
	}


	tessellatesurfacemy->SetInt("numControlPointsU", NumControlVertices);
	tessellatesurfacemy->SetInt("numControlPointsV", NumControlVertices);
	tessellatesurfacemy->SetInt("degreeU", current.degree);
	tessellatesurfacemy->SetInt("degreeV", current.degree);
	tessellatesurfacemy->SetFloatArray("knotsU", current.knots, NumControlVertices + current.degree + 1);
	tessellatesurfacemy->SetFloatArray("knotsV", current.knots, NumControlVertices + current.degree + 1);
	tessellatesurfacemy->SetFloatArray("weightsU", current.weights, NumControlVertices);
	tessellatesurfacemy->SetFloatArray("weightsV", current.weights, NumControlVertices);
	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], NumControlVertices * NumControlVertices);

	tessellatesurfacemy->Begin();
	{
		glDispatchCompute(1, 1, 1);
	}
	tessellatesurfacemy->End();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT|GL_ELEMENT_ARRAY_BARRIER_BIT);

	int numSegmentsU = numSegBetweenKnotsU * (NumControlVertices - current.degree);
	int numSegmentsV = numSegBetweenKnotsV * (NumControlVertices - current.degree);
	if (current.degree > 1) {
		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
	} else {
//		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
	}

	delete[] surfacecvs;
}

void KeyUp(KeyCode key)
{
	for (int i = 0; i < ARRAY_SIZE(curves); ++i) {
		if (key == KeyCode1 + i) {
			Tessellate();
		}
	}

	switch (key) {

	case KeyCodeW:
		wireframe = !wireframe;
		break;

	case KeyCodeA:
		numSegBetweenKnotsU = Math::Min<int>(numSegBetweenKnotsU + 2, 20);
		numSegBetweenKnotsV = Math::Min<int>(numSegBetweenKnotsV + 2, 20);
		Tessellate();
		break;

	case KeyCodeD:
		numSegBetweenKnotsU = Math::Max<int>(numSegBetweenKnotsU - 2, 2);
		numSegBetweenKnotsV = Math::Max<int>(numSegBetweenKnotsV - 2, 2);
		Tessellate();
		break;

	default:
		break;
	}
}

void MouseMove(int32_t x, int32_t y, int16_t dx, int16_t dy)
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();
	uint8_t state = app->GetMouseButtonState();

	if (state & MouseButtonLeft) {
		int left = 0;
		int right = screenwidth;
		int top = 0;
		int bottom = screenheight;

		if (((x >= left && x <= right) &&
			(y >= top && y <= bottom)))
		{
			camera.OrbitRight(Math::DegreesToRadians(dx));
			camera.OrbitUp(Math::DegreesToRadians(dy));
		}
	}
}

void Update(float delta)
{
	camera.Update(delta);
}

void Render(float alpha, float elapsedtime)
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	Math::Matrix world, view, proj;
	Math::Matrix viewproj;

	Math::Vector4 lightdir			= { 0, 1, 0, 0 };
	Math::Vector3 eye;

	Math::Color	outsidecolor(0.75f, 0.75f, 0.8f, 1);
	Math::Color	insidecolor(1, 0.66f, 0.066f, 1);

	// render grid, control polygon and curve
	Math::MatrixIdentity(world);

	glClearColor(0.0f, 0.125f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

	glEnable(GL_DEPTH_TEST);

	glViewport(0, 0, screenwidth, screenheight);

	camera.Animate(alpha);
	camera.SetAspect((float)screenwidth / screenheight);
	camera.GetViewMatrix(view);
	camera.GetEyePosition(eye);
	camera.GetProjectionMatrix(proj);

	Math::MatrixMultiply(viewproj, view, proj);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glDisable(GL_CULL_FACE);

	rendersurface->SetMatrix("matViewProj", viewproj);
	rendersurface->SetMatrix("matWorld", world);
	rendersurface->SetMatrix("matWorldInv", world);
	rendersurface->SetVector("lightDir", lightdir);
	rendersurface->SetVector("eyePos", eye);
	rendersurface->SetVector("outsideColor", outsidecolor);
	rendersurface->SetVector("insideColor", insidecolor);
	rendersurface->SetInt("isWireMode", wireframe);

	rendersurface->Begin();
	{
		surface->DrawSubset(0);
	}
	rendersurface->End();

	glEnable(GL_CULL_FACE);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// reset states
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glViewport(0, 0, screenwidth, screenheight);

	// check errors
	GLenum err = glGetError();

	if (err != GL_NO_ERROR)
		std::cout << "Error\n";

	app->Present();
}

int main(int argc, char* argv[])
{
	app = Application::Create(768, 768);
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
