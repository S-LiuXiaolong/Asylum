
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#include <iostream>
#include <sstream>
#include <fstream>

#include "application.h"
#include "gl4ext.h"
#include "basiccamera.h"

// helper macros
#define TITLE				"NURBS tessellation"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

#define DEGREE 2

// FIXME: Delete
// control constants
#define MAX_NUM_SEGMENTS 100
const GLuint NumControlVertices				= 7;
const GLuint NumControlIndices				= (NumControlVertices - 1) * 2;
const GLuint MaxSplineVertices				= MAX_NUM_SEGMENTS + 1;
const GLuint MaxSplineIndices				= (MaxSplineVertices - 1) * 2;
const GLuint MaxSurfaceVertices				= MaxSplineVertices * MaxSplineVertices;
const GLuint MaxSurfaceIndices				= (MaxSplineVertices - 1) * (MaxSplineVertices - 1) * 6;

// FIXME: Add a more flexible way to define a CurveData but not hardcode.
// // sample structures
// struct CurveData
// {
// 	int degree;	// max 3
// 	Math::Vector4 controlpoints[NumControlVertices];
// 	float weights[NumControlVertices + 4];
// 	float knots[11];
// };

// TODO: A SurfaceData may be useful? CurveData is useless in my situation.
// TODO: Maybe put the surface and whole-mesh class into another file?
struct NURBSSurfaceData
{
	std::vector<std::vector<uint32_t>> cptsIndex;
	std::vector<std::vector<float>> weights;
	std::vector<float> knotU, knotV;
};

NURBSSurfaceData mesh_surfaces[6];

// sample variables
Application*		app					= nullptr;

OpenGLEffect*		rendersurface		= nullptr;
OpenGLScreenQuad*	screenquad			= nullptr;
std::vector<OpenGLMesh*>		surfacegroup;

BasicCamera			camera;
float				selectiondx			= 0;
float				selectiondy			= 0;
int					numSegBetweenKnotsU = 2;
int					numSegBetweenKnotsV = 2;
bool				wireframe			= false;

void Tessellate();

std::vector<Math::Vector3> mesh_cp_vertices;
std::vector<float> mesh_cp_weights;
uint32_t nelx, nely, nelz;
uint32_t numCptx, numCpty, numCptz;
std::vector<float> knotx, knoty, knotz;
std::vector<std::vector<std::vector<uint32_t>>> chan;

// TODO: Maybe put these functions into another utility file?
void read_float(std::string strFile, std::vector<float>& buffer);
void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer);

void build_surface()
{
	std::vector<std::vector<uint32_t>> faceIndices[6];
	
	faceIndices[0] = chan[0];
	faceIndices[1] = chan[numCptx - 1];
	
	for (int i = 0; i < numCptx; i++)
	{
		std::vector<uint32_t> onerow1 = chan[i][0];
		faceIndices[2].push_back(onerow1);

		std::vector<uint32_t> onerow2 = chan[i][numCptz - 1];
		faceIndices[3].push_back(onerow2);

		std::vector<uint32_t> onecolumn1;
		for (int j = 0; j < numCptz; j++)
		{
			onecolumn1.push_back(chan[i][j][0]);
		}
		faceIndices[4].push_back(onecolumn1);

		std::vector<uint32_t> onecolumn2;
		for (int j = 0; j < numCptz; j++)
		{
			onecolumn2.push_back(chan[i][j][numCpty - 1]);
		}
		faceIndices[5].push_back(onecolumn2);
	}

	for (int i = 0; i < 6; i++)
	{
		auto& faceIndex = faceIndices[i];
		int numCptsU = faceIndex.size(); int numCptsV = faceIndex[0].size();
		std::vector<std::vector<Math::Vector3>> cpts(numCptsU, std::vector<Math::Vector3>(numCptsV, { 0,0,0 }));
		std::vector<std::vector<float>> wts(numCptsU, std::vector<float>(numCptsV, 0));

		for (int axisU = 0; axisU < numCptsU; axisU++)
		{
			for (int axisV = 0; axisV < numCptsV; axisV++)
			{
				// Here we do not pass cpts pos into surfaceData to avoid rebuild surface when moving control points.
				// cpts[axisU][axisV] = mesh_cp_vertices[faceIndex[axisU][axisV] - 1];
				wts[axisU][axisV] = mesh_cp_weights[faceIndex[axisU][axisV] - 1];
			}
		}

		std::vector<float> knotu, knotv;
		switch (i) {
		case 0: 
		case 1: 
			knotu = knotz; knotv = knoty; break;
		case 2:
		case 3:
			knotu = knotx; knotv = knoty; break;
		case 4:
		case 5:
			knotu = knotx; knotv = knotz; break;
		}
		
		mesh_surfaces[i] = { faceIndex, wts, knotu, knotv };
	}
}

void build_mesh()
{
	// Get all coords of points.
	std::vector<float> buffer_cpts;
	read_float("../../../Asset/controlPts.bin", buffer_cpts);

	mesh_cp_vertices.resize(buffer_cpts.size() / 3);

	for (int i = 0; i < buffer_cpts.size() / 3; i++)
	{
		mesh_cp_vertices[i] = { buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2] };
		// printf("%f %f %f\n", buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2]);
	}
	// For testing deformation
	// mesh_cp_vertices[10].z -= 1.0f;
	// mesh_cp_vertices[19].z -= 0.5f;
	// mesh_cp_vertices[19].x += 0.5f;

	// Get the nels(but what is nel?) and numControlPts. numCpts = nel + DEGREE.
	std::vector<uint32_t> buffer_nels;
	read_uint32t("../../../Asset/nels.bin", buffer_nels);
	nelx = buffer_nels[0]; nely = buffer_nels[1]; nelz = buffer_nels[2];
	numCptx = buffer_nels[0] + DEGREE; numCpty = buffer_nels[1] + DEGREE; numCptz = buffer_nels[2] + DEGREE;

	// FIXME: SHITCODE. Weight is hardcoded as 1.
	// Get all weights(same size as the controlPts) from binary file.
	read_float("../../../Asset/weights.bin", mesh_cp_weights);

	// Get xyz knots from binary file.
	std::vector<float> buffer_knots;
	read_float("../../../Asset/knots.bin", buffer_knots);
	int rowLength = buffer_knots.size() / 3;
	
	// FIXME: SHITCODE. Please write in a more clear way.
	auto knotxBegin = buffer_knots.begin(); auto knotxEnd = buffer_knots.begin() + numCptx + 2 + 1;
	auto knotyBegin = buffer_knots.begin() + rowLength; auto knotyEnd = buffer_knots.begin() + rowLength + numCpty + 2 + 1;
	auto knotzBegin = buffer_knots.begin() + rowLength * 2; auto knotzEnd = buffer_knots.begin() + rowLength * 2 + numCptz + 2 + 1;
	knotx.assign(knotxBegin, knotxEnd);
	knoty.assign(knotyBegin, knotyEnd);
	knotz.assign(knotzBegin, knotzEnd);

	// Get chan(but what is chan?) from binary file.
	std::vector<uint32_t> buffer_chan;
	read_uint32t("../../../Asset/chan.bin", buffer_chan);
	for (int i = 0; i < numCptx; i++)
	{
		std::vector<std::vector<uint32_t>> face;
		for (int j = 0; j < numCptz; j++)
		{
			std::vector<uint32_t> line;
			for (int k = 0; k < numCpty; k++)
			{
				line.push_back(buffer_chan[i * numCptz * numCpty + j * numCpty + k]);
			}
			face.push_back(line);
		}
		chan.push_back(face);
	}

	build_surface();
}

void read_float(std::string strFile, std::vector<float>& buffer)
{
	float temp;
	std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
		return;
	}

	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

	while (infile.read((char*)&temp, sizeof(float)))
	{
		int readedBytes = infile.gcount();
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}
}

void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer)
{
	uint32_t temp;
	std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
		return;
	}

	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

	while (infile.read((char*)&temp, sizeof(uint32_t)))
	{
		int readedBytes = infile.gcount();
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}
}

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

	build_mesh();

	// load shaders
	// TODO: Add a more clear way (assetloader)
	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/rendersurface.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/rendersurface.frag", &rendersurface)) {
		MYERROR("Could not load surface renderer shader");
		return false;
	}

	screenquad = new OpenGLScreenQuad();

	// tessellate for the first time
	Tessellate();

	// setup camera
	camera.SetAspect((float)screenwidth / screenheight);
	camera.SetFov(Math::DegreesToRadians(60));
	camera.SetClipPlanes(0.1f, 50.0f);
	camera.SetDistance(5);
	camera.SetOrientation(Math::HALF_PI, 0.5f, 0);
	camera.SetPosition(1.5, 0.5, 0.1);

	return true;
}

void UninitScene()
{
	delete rendersurface;
	delete screenquad;

	OpenGLContentManager().Release();
}

void Tessellate()
{
	OpenGLVertexElement decl[] = {
		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	for(int i = 0; i < 6; i++)
	{
		NURBSSurfaceData surfData = mesh_surfaces[i];
		auto& cptsIndex = surfData.cptsIndex;
		auto& weights = surfData.weights;
		auto& knotU = surfData.knotU;
		auto& knotV = surfData.knotV;

		int numCptU = cptsIndex.size();
		int numCptV = cptsIndex[0].size();

		OpenGLMesh* surface = nullptr;
		OpenGLEffect* tessellatesurfacemy = nullptr;

		// create surface
		// FIXME: How to get MaxSurfaceVertices and Indices?
		if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl, &surface)) {
			MYERROR("Could not create surface");
			return;
		}

		if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
			MYERROR("Could not load compute shader");
			return;
		}

		// update surface cvs and weights (STL 2D vector will cause fault)
		Math::Vector4* surfacecvs = new Math::Vector4[numCptU * numCptV];
		float* surfacewts = new float[numCptU * numCptV];
		GLuint index;

		for (GLuint m = 0; m < numCptU; ++m) {
			for (GLuint n = 0; n < numCptV; ++n) {
				index = m * numCptV + n;
				surfacecvs[index][0] = mesh_cp_vertices[cptsIndex[m][n] - 1].x;
				surfacecvs[index][1] = mesh_cp_vertices[cptsIndex[m][n] - 1].y;
				surfacecvs[index][2] = mesh_cp_vertices[cptsIndex[m][n] - 1].z;
				// std::cout << mesh_cp_vertices[cptsIndex[m][n] - 1].x << mesh_cp_vertices[cptsIndex[m][n] - 1].y << mesh_cp_vertices[cptsIndex[m][n] - 1].z << std::endl;
				surfacewts[index] = weights[m][n];
			}
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());

		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);

		// FIXME: SHITCODE. Please put these variables in a manner way.
		tessellatesurfacemy->SetInt("numControlPointsU", numCptU);
		tessellatesurfacemy->SetInt("numControlPointsV", numCptV);
		tessellatesurfacemy->SetInt("degreeU", DEGREE);
		tessellatesurfacemy->SetInt("degreeV", DEGREE);
		tessellatesurfacemy->SetFloatArray("knotsU", &knotU[0], numCptU + DEGREE + 1);
		tessellatesurfacemy->SetFloatArray("knotsV", &knotV[0], numCptV + DEGREE + 1);
		// FIXME: weight should be passed into compute shader in the same way as controlPoints.
		// tessellatesurfacemy->SetFloatArray("weightsu", weight1, numCptU);
		// tessellatesurfacemy->SetFloatArray("weightsv", weight2, numCptV);
		tessellatesurfacemy->SetFloatArray("weights", &surfacewts[0], numCptU * numCptV);
		tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], numCptU * numCptV);

		tessellatesurfacemy->Begin();
		{
			glDispatchCompute(1, 1, 1);
		}
		tessellatesurfacemy->End();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);

		int numSegmentsU = numSegBetweenKnotsU * (numCptU - DEGREE);
		int numSegmentsV = numSegBetweenKnotsV * (numCptV - DEGREE);

		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;

		delete[] surfacecvs;

		delete tessellatesurfacemy;

		surfacegroup.push_back(surface);
	}

}


void KeyUp(KeyCode key)
{
	// for (int i = 0; i < ARRAY_SIZE(curves); ++i) {
	// 	if (key == KeyCode1 + i) {
	// 		// Tessellate();
	// 	}
	// }

	switch (key) {

	case KeyCodeW:
		wireframe = !wireframe;
		break;

	case KeyCodeA:
		numSegBetweenKnotsU = Math::Min<int>(numSegBetweenKnotsU + 2, 20);
		numSegBetweenKnotsV = Math::Min<int>(numSegBetweenKnotsV + 2, 20);
		// Tessellate();
		break;

	case KeyCodeD:
		numSegBetweenKnotsU = Math::Max<int>(numSegBetweenKnotsU - 2, 2);
		numSegBetweenKnotsV = Math::Max<int>(numSegBetweenKnotsV - 2, 2);
		// Tessellate();
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

	Math::Vector4 lightdir			= { 1, 1, -1, 0 };
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
		// surface->DrawSubset(0);
		for (int i = 0; i < 6; i++)
		{
			surfacegroup[i]->DrawSubset(0);
		}
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
