
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

#define MAX_NUM_SEGMENTS	100

// helper macros
#define TITLE				"NURBS tessellation"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// FIXME: Delete
// control constants
const GLuint NumControlVertices				= 7;
const GLuint NumControlIndices				= (NumControlVertices - 1) * 2;
const GLuint MaxSplineVertices				= MAX_NUM_SEGMENTS + 1;
const GLuint MaxSplineIndices				= (MaxSplineVertices - 1) * 2;
const GLuint MaxSurfaceVertices				= MaxSplineVertices * MaxSplineVertices;
const GLuint MaxSurfaceIndices				= (MaxSplineVertices - 1) * (MaxSplineVertices - 1) * 6;

// FIXME: Add a more flexible way to define a CurveData but not hardcode.
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

OpenGLEffect*		rendersurface		= nullptr;
OpenGLScreenQuad*	screenquad			= nullptr;
std::vector<OpenGLMesh*>		surfacegroup;

BasicCamera			camera;
float				selectiondx			= 0;
float				selectiondy			= 0;
// int					numsegments			= MAX_NUM_SEGMENTS / 2;
int					numSegBetweenKnotsU = 5;
int					numSegBetweenKnotsV = 5;
// int					currentcurve		= 0;
bool				wireframe			= false;

void Tessellate(std::vector<std::vector<uint32_t>> faceindex);

#define DEGREE 2

struct vertex { GLfloat x, y, z; };
std::vector<vertex> mesh_cp_vertices;
uint32_t nelx, nely, nelz;
uint32_t numCptx, numCpty, numCptz;
std::vector<float> knotx, knoty, knotz;
std::vector<std::vector<std::vector<uint32_t>>> chan;
std::vector<float> weightx;
std::vector<float> weighty;
std::vector<float> weightz;

void read_float(std::string strFile, std::vector<float>& buffer);
void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer);

void build_mesh()
{
	std::vector<float> buffer_cpts;
	read_float("../../../Asset/controlPts.bin", buffer_cpts);

	mesh_cp_vertices.resize(buffer_cpts.size() / 3);

	for (int i = 0; i < buffer_cpts.size() / 3; i++)
	{
		mesh_cp_vertices[i] = { buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2] };
		// printf("%f %f %f\n", buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2]);
	}
	// for debug
	mesh_cp_vertices[10].z -= 1.0f;
	// mesh_cp_vertices[19].z -= 0.5f;
	// mesh_cp_vertices[19].x += 0.5f;


	std::vector<uint32_t> buffer_nels;
	read_uint32t("../../../Asset/nels.bin", buffer_nels);
	nelx = buffer_nels[0]; nely = buffer_nels[1]; nelz = buffer_nels[2];
	numCptx = buffer_nels[0] + DEGREE; numCpty = buffer_nels[1] + DEGREE; numCptz = buffer_nels[2] + DEGREE;

	// FIXME: SHITCODE. Weight is hardcoded as 1.
	weightx.resize(numCptx);
	for (int i = 0; i < numCptx; i++) {
		weightx[i] = 1;
	}
	weighty.resize(numCpty);
	for (int i = 0; i < numCpty; i++) {
		weighty[i] = 1;
	}
	weightz.resize(numCptz);
	for (int i = 0; i < numCptz; i++) {
		weightz[i] = 1;
	}


	std::vector<float> buffer_knots;
	read_float("../../../Asset/knots.bin", buffer_knots);
	int rowLength = buffer_knots.size() / 3;
	
	// FIXME: SHITCODE. Please write in a more clear way.
	knotx.assign(buffer_knots.begin(), buffer_knots.begin() + numCptx + 2 + 1);
	knoty.assign(buffer_knots.begin() + rowLength, buffer_knots.begin() + rowLength + numCpty + 2 + 1);
	knotz.assign(buffer_knots.begin() + rowLength * 2, buffer_knots.begin() + rowLength * 2 + numCptz + 2 + 1);

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

	// FIXME: SHITCODE. Absolutely shitcode. faceIndex may need to be put into global.
	std::vector<std::vector<std::vector<uint32_t>>> facesIndex;
	std::vector<std::vector<uint32_t>> face1, face2, face3, face4, face5, face6;
	
	face1 = chan[0];
	facesIndex.push_back(face1);
	face2 = chan[numCptx - 1];
	facesIndex.push_back(face2);
	for (int i = 0; i < numCptx; i++)
	{
		std::vector<uint32_t> onerow = chan[i][0];
		face3.push_back(onerow);
	}
	facesIndex.push_back(face3);

	for (int i = 0; i < numCptx; i++)
	{
		std::vector<uint32_t> onerow = chan[i][numCptz - 1];
		face4.push_back(onerow);
	}
	facesIndex.push_back(face4);

	for (int i = 0; i < numCptx; i++)
	{
		std::vector<uint32_t> onecolumn;
		for (int j = 0; j < numCptz; j++)
		{
			onecolumn.push_back(chan[i][j][0]);
		}
		face5.push_back(onecolumn);
	}
	facesIndex.push_back(face5);

	for (int i = 0; i < numCptx; i++)
	{
		std::vector<uint32_t> onecolumn;
		for (int j = 0; j < numCptz; j++)
		{
			onecolumn.push_back(chan[i][j][numCpty - 1]);
		}
		face6.push_back(onecolumn);
	}
	facesIndex.push_back(face6);


	// tessellate for the first time
	for (int i = 0; i < 6; i++)
	{

		// Tessellate(int index, int currentCurve);
		Tessellate(facesIndex[i]);
	}
	// Tessellate();

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

// void Tessellate()
// void Tessellate(int count, int currentcurve)
// {
// 	OpenGLMesh* surface = nullptr;
// 	OpenGLEffect* tessellatesurfacemy = nullptr;
// 
// 	OpenGLVertexElement decl2[] = {
// 		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
// 		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
// 		{ 0xff, 0, 0, 0, 0 }
// 	};
// 
// 	// create surface
// 	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
// 		MYERROR("Could not create surface");
// 		return ;
// 	}
// 
// 	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
// 		MYERROR("Could not load compute shader");
// 		return ;
// 	}
// 
// 	CurveData& current = curves[currentcurve];
// 
// 	// update surface cvs
// 	Math::Vector4* surfacecvs = new Math::Vector4[NumControlVertices * NumControlVertices];
// 	GLuint index;
// 
// 	for (GLuint i = 0; i < NumControlVertices; ++i) {
// 		for (GLuint j = 0; j < NumControlVertices; ++j) {
// 			index = i * NumControlVertices + j;
// 
// 			surfacecvs[index][0] = current.controlpoints[i][0];
// 			surfacecvs[index][2] = current.controlpoints[j][0];
// 			surfacecvs[index][1] = (current.controlpoints[i][1] + current.controlpoints[j][1]) * 0.5f;
// 		}
// 	}
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());
// 
// // 	if (current.degree > 1) {
// // 		tessellatesurfacemy->SetInt("numVerticesU", numsegments + 1);
// // 		tessellatesurfacemy->SetInt("numVerticesV", numsegments + 1);
// // 	} else {
// // 		tessellatesurfacemy->SetInt("numVerticesU", NumControlVertices);
// // 		tessellatesurfacemy->SetInt("numVerticesV", NumControlVertices);
// // 	}
// 
// 	if (current.degree > 1) {
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// 	} else {
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
// 	}
// 
// 
// 	tessellatesurfacemy->SetInt("numControlPointsU", NumControlVertices);
// 	tessellatesurfacemy->SetInt("numControlPointsV", NumControlVertices);
// 	tessellatesurfacemy->SetInt("degreeU", current.degree);
// 	tessellatesurfacemy->SetInt("degreeV", current.degree);
// 	tessellatesurfacemy->SetFloatArray("knotsU", current.knots, NumControlVertices + current.degree + 1);
// 	tessellatesurfacemy->SetFloatArray("knotsV", current.knots, NumControlVertices + current.degree + 1);
// 	tessellatesurfacemy->SetFloatArray("weightsU", current.weights, NumControlVertices);
// 	tessellatesurfacemy->SetFloatArray("weightsV", current.weights, NumControlVertices);
// 	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], NumControlVertices * NumControlVertices);
// 
// 	tessellatesurfacemy->Begin();
// 	{
// 		glDispatchCompute(1, 1, 1);
// 	}
// 	tessellatesurfacemy->End();
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
// 
// 	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT|GL_ELEMENT_ARRAY_BARRIER_BIT);
// 
// 	int numSegmentsU = numSegBetweenKnotsU * (NumControlVertices - current.degree);
// 	int numSegmentsV = numSegBetweenKnotsV * (NumControlVertices - current.degree);
// 	if (current.degree > 1) {
// 		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// 	} else {
// //		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
// 	}
// 
// 	delete[] surfacecvs;
// 
// 	surfacegroup.push_back(surface);
// }

// void Tessellate(int count, int currentcurve)
// {
// 	auto& faceindex = chan[0];
// 
// 	OpenGLMesh* surface = nullptr;
// 	OpenGLEffect* tessellatesurfacemy = nullptr;
// 
// 	OpenGLVertexElement decl2[] = {
// 		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
// 		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
// 		{ 0xff, 0, 0, 0, 0 }
// 	};
// 
// 	// create surface
// 	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
// 		MYERROR("Could not create surface");
// 		return;
// 	}
// 
// 	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
// 		MYERROR("Could not load compute shader");
// 		return;
// 	}
// 
// 	// CurveData& current = curves[currentcurve];
// 
// 	// update surface cvs
// 	Math::Vector4* surfacecvs = new Math::Vector4[6 * 5];
// 	GLuint index;
// 
// 	for (GLuint i = 0; i < 6; ++i) {
// 		for (GLuint j = 0; j < 5; ++j) {
// 			index = i * 5 + j;
// 
// 			surfacecvs[index][0] = mesh_cp_vertices[faceindex[i][j] - 1].x;
// 			surfacecvs[index][2] = mesh_cp_vertices[faceindex[i][j] - 1].z;
// 			surfacecvs[index][1] = mesh_cp_vertices[faceindex[i][j] - 1].y;
// 		}
// 	}
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());
// 
// 	// 	if (current.degree > 1) {
// 	// 		tessellatesurfacemy->SetInt("numVerticesU", numsegments + 1);
// 	// 		tessellatesurfacemy->SetInt("numVerticesV", numsegments + 1);
// 	// 	} else {
// 	// 		tessellatesurfacemy->SetInt("numVerticesU", NumControlVertices);
// 	// 		tessellatesurfacemy->SetInt("numVerticesV", NumControlVertices);
// 	// 	}
// 
// // 	if (current.degree > 1) {
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// // 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// // 	}
// // 	else {
// // 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// // 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
// // 	}
// 
// 	tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// 	tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// 
// 
// 	tessellatesurfacemy->SetInt("numControlPointsU", 6);
// 	tessellatesurfacemy->SetInt("numControlPointsV", 5);
// 	tessellatesurfacemy->SetInt("degreeU", 2);
// 	tessellatesurfacemy->SetInt("degreeV", 2);
// 	tessellatesurfacemy->SetFloatArray("knotsU", &knotz[0], 6 + 2 + 1);
// 	tessellatesurfacemy->SetFloatArray("knotsV", &knoty[0], 5 + 2 + 1);
// 	tessellatesurfacemy->SetFloatArray("weightsU", weightz, 6);
// 	tessellatesurfacemy->SetFloatArray("weightsV", weighty, 5);
// 	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], 6 * 5);
// 
// 	tessellatesurfacemy->Begin();
// 	{
// 		glDispatchCompute(1, 1, 1);
// 	}
// 	tessellatesurfacemy->End();
// 
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
// 	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
// 
// 	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);
// 
// 	int numSegmentsU = numSegBetweenKnotsU * (6 - 2);
// 	int numSegmentsV = numSegBetweenKnotsV * (5 - 2);
// // 	if (current.degree > 1) {
// // 		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// // 	}
// // 	else {
// // 		//		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
// // 	}
// 	surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// 
// 	delete[] surfacecvs;
// 
// 	surfacegroup.push_back(surface);
// }

void Tessellate(std::vector<std::vector<uint32_t>> faceindex)
{
	OpenGLMesh* surface = nullptr;
	OpenGLEffect* tessellatesurfacemy = nullptr;

	// FIXME: SHITCODE.
	float* weight1 = nullptr;
	float* weight2 = nullptr;
	std::vector<float> knot1;
	std::vector<float> knot2;

	if (faceindex.size() == numCptx) {
		weight1 = &weightx[0];
		knot1 = knotx;
	}
	else if (faceindex.size() == numCpty) {
		weight1 = &weighty[0];
		knot1 = knoty;
	}
	else if (faceindex.size() == numCptz) {
		weight1 = &weightz[0];
		knot1 = knotz;
	}

	if (faceindex[0].size() == numCptx) {
		weight2 = &weightx[0];
		knot2 = knotx;
	}
	else if (faceindex[0].size() == numCpty) {
		weight2 = &weighty[0];
		knot2 = knoty;
	}
	else if (faceindex[0].size() == numCptz) {
		weight2 = &weightz[0];
		knot2 = knotz;
	}

	OpenGLVertexElement decl2[] = {
		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	// create surface
	if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl2, &surface)) {
		MYERROR("Could not create surface");
		return;
	}

	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemy.comp", &tessellatesurfacemy)) {
		MYERROR("Could not load compute shader");
		return;
	}

	// CurveData& current = curves[currentcurve];

	// update surface cvs
	Math::Vector4* surfacecvs = new Math::Vector4[faceindex.size() * faceindex[0].size()];
	GLuint index;

	for (GLuint i = 0; i < faceindex.size(); ++i) {
		for (GLuint j = 0; j < faceindex[0].size(); ++j) {
			index = i * faceindex[0].size() + j;
			surfacecvs[index][0] = mesh_cp_vertices[faceindex[i][j] - 1].x;
			surfacecvs[index][1] = mesh_cp_vertices[faceindex[i][j] - 1].y;
			surfacecvs[index][2] = mesh_cp_vertices[faceindex[i][j] - 1].z;
			// std::cout << mesh_cp_vertices[faceindex[i][j] - 1].x << mesh_cp_vertices[faceindex[i][j] - 1].y << mesh_cp_vertices[faceindex[i][j] - 1].z << std::endl;
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

// 	if (current.degree > 1) {
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
// 	}
// 	else {
// 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", NumControlVertices);
// 		// 		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", NumControlVertices);
// 	}

	tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
	tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);

	// FIXME: SHITCODE. Please put these variables in a manner way.
	tessellatesurfacemy->SetInt("numControlPointsU", faceindex.size());
	tessellatesurfacemy->SetInt("numControlPointsV", faceindex[0].size());
	tessellatesurfacemy->SetInt("degreeU", 2);
	tessellatesurfacemy->SetInt("degreeV", 2);
	tessellatesurfacemy->SetFloatArray("knotsU", &knot1[0], faceindex.size() + 2 + 1);
	tessellatesurfacemy->SetFloatArray("knotsV", &knot2[0], faceindex[0].size() + 2 + 1);
	tessellatesurfacemy->SetFloatArray("weightsU", weight1, faceindex.size());
	tessellatesurfacemy->SetFloatArray("weightsV", weight2, faceindex[0].size());
	tessellatesurfacemy->SetVectorArray("controlPoints", &surfacecvs[0][0], faceindex.size() * faceindex[0].size());

	tessellatesurfacemy->Begin();
	{
		glDispatchCompute(1, 1, 1);
	}
	tessellatesurfacemy->End();

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);

	int numSegmentsU = numSegBetweenKnotsU * (faceindex.size() - 1);
	int numSegmentsV = numSegBetweenKnotsV * (faceindex[0].size() - 1);

// 	if (current.degree > 1) {
// 		surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;
// 	}
// 	else {
// 		//		surface->GetAttributeTable()->IndexCount = (NumControlVertices - 1) * (NumControlVertices - 1) * 6;
// 	}

	surface->GetAttributeTable()->IndexCount = numSegmentsU * numSegmentsV * 6;

	delete[] surfacecvs;

	delete tessellatesurfacemy;

	surfacegroup.push_back(surface);
}


void KeyUp(KeyCode key)
{
	for (int i = 0; i < ARRAY_SIZE(curves); ++i) {
		if (key == KeyCode1 + i) {
			// Tessellate();
		}
	}

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
