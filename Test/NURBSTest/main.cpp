
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
#define TITLE				"NURBS tesselation"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

#define DEGREE 2

// FIXME: Delete
// control constants
#define MAX_NUM_SEGMENTS 1000
const GLuint NumControlVertices				= 7;
const GLuint NumControlIndices				= (NumControlVertices - 1) * 2;
const GLuint MaxSplineVertices				= MAX_NUM_SEGMENTS + 1;
const GLuint MaxSplineIndices				= (MaxSplineVertices - 1) * 4;
const GLuint MaxSurfaceVertices				= MaxSplineVertices * MaxSplineVertices;
const GLuint MaxSurfaceIndices				= (MaxSplineVertices - 1) * (MaxSplineVertices - 1) * 6;

// TODO: Maybe put the surface and whole-mesh class into another file?
struct NURBSSurfaceData
{
	std::vector<std::vector<uint32_t>> cptsIndex;
	std::vector<std::vector<float>> weights;
	std::vector<float> knotU, knotV;

	std::vector<std::vector<float>> patchRho;
};

// FIXME: I need more thinking about NURBS
struct NURBSLayerData
{
	std::vector<std::vector<std::vector<uint32_t>>> cptsIndex;
	std::vector<std::vector<std::vector<float>>> weights;
	std::vector<float> knotU, knotV, knotW;

	std::vector<std::vector<std::vector<float>>> LayerRho;
};

// NURBSSurfaceData mesh_surfaces[6];
std::vector<NURBSSurfaceData> mesh_surfaces;
// TODO
NURBSLayerData mesh_layers[3];

// sample variables
Application*		app						= nullptr;

OpenGLEffect*		rendersurface			= nullptr;
OpenGLEffect*		tessellatesurfacemy		= nullptr;
OpenGLScreenQuad*	screenquad				= nullptr;
uint32_t cptsBuffer, wtsBuffer, rhoBuffer;

std::vector<OpenGLMesh*>		surfacegroup;

BasicCamera			camera;
float				selectiondx			= 0;
float				selectiondy			= 0;
int					numSegBetweenKnotsU = 1;
int					numSegBetweenKnotsV = 1;
bool				wireframe			= false;

void Tessellate();

std::vector<Math::Vector3> mesh_cp_vertices;
std::vector<float> mesh_cp_weights;
uint32_t nelx, nely, nelz;
uint32_t numCptx, numCpty, numCptz;
std::vector<float> knotx, knoty, knotz;
std::vector<std::vector<std::vector<uint32_t>>> chan;
std::vector<std::vector<std::vector<float>>> mesh_rho;

// TODO: Maybe put these functions into another utility file?
void read_float(std::string strFile, std::vector<float>& buffer);
void read_uint32t(std::string strFile, std::vector<uint32_t>& buffer);

void build_surface()
{
	mesh_layers[0].cptsIndex.resize(numCptx);
	for(int i = 0; i < numCptx; i++)
	{
		mesh_layers[0].cptsIndex[i] = chan[i];
	}

	mesh_layers[1].cptsIndex.resize(numCptz);
	for(int i = 0; i < numCptz; i++)
	{
		for(int j = 0; j < numCptx; j++)
		{
			std::vector<uint32_t> onerow = chan[j][i];
			mesh_layers[1].cptsIndex[i].push_back(onerow);
		}
	}

	mesh_layers[2].cptsIndex.resize(numCpty);
	for(int i = 0; i < numCpty; i++)
	{
		for(int j = 0; j < numCptx; j++)
		{
			std::vector<uint32_t> onecolomn;
			for(int k = 0; k < numCptz; k++)
			{
				onecolomn.push_back(chan[j][k][i]);
			}
			mesh_layers[2].cptsIndex[i].push_back(onecolomn);
		}
	}

	// Element density
	mesh_layers[0].LayerRho.resize(nelx);
	for(int i = 0; i < nelx; i++)
	{
		mesh_layers[0].LayerRho[i] = mesh_rho[i];
	}

	mesh_layers[1].LayerRho.resize(nelz);
	for(int i = 0; i < nelz; i++)
	{
		for(int j = 0; j < nelx; j++)
		{
			std::vector<float> onerow = mesh_rho[j][i];
			mesh_layers[1].LayerRho[i].push_back(onerow);
		}
	}

	mesh_layers[2].LayerRho.resize(nely);
	for(int i = 0; i < nely; i++)
	{
		for(int j = 0; j < nelx; j++)
		{
			std::vector<float> onecolomn;
			for(int k = 0; k < nelz; k++)
			{
				onecolomn.push_back(mesh_rho[j][k][i]);
			}
			mesh_layers[2].LayerRho[i].push_back(onecolomn);
		}
	}

	for(int i = 0; i < 3; i++)
	{
		auto& cptsIndex = mesh_layers[i].cptsIndex;
		
		int numCptsU = cptsIndex[0].size();
		int numCptsV = cptsIndex[0][0].size();
		int numCptsW = cptsIndex.size();

		std::vector<std::vector<std::vector<float>>> wts(
			numCptsW, std::vector<std::vector<float>>(numCptsU, std::vector<float>(numCptsV, 0)));

		for(int axisW = 0; axisW < numCptsW; axisW++)
		{
			for (int axisU = 0; axisU < numCptsU; axisU++)
			{
				for (int axisV = 0; axisV < numCptsV; axisV++)
				{
					// Here we do not pass cpts pos into surfaceData to avoid rebuild surface when moving control points.
					// cpts[axisU][axisV] = mesh_cp_vertices[faceIndex[axisU][axisV] - 1];
					wts[axisW][axisU][axisV] = mesh_cp_weights[cptsIndex[axisW][axisU][axisV] - 1];
				}
			}
		}
		mesh_layers[i].weights = wts;

		std::vector<float> knotu, knotv, knotw;
		switch (i) {
		case 0: 
			knotu = knotz; knotv = knoty; knotw = knotx; break;
		case 1:
			knotu = knotx; knotv = knoty; knotw = knotz; break;
		case 2:
			knotu = knotx; knotv = knotz; knotw = knoty; break;
		}
		mesh_layers[i].knotU = knotu;
		mesh_layers[i].knotV = knotv;
		mesh_layers[i].knotW = knotw;
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
	}

	// Get the nels(but what is nel?) and numControlPts. numCpts = nel + DEGREE.
	std::vector<uint32_t> buffer_nels;
	read_uint32t("../../../Asset/nels.bin", buffer_nels);
	nelx = buffer_nels[0]; nely = buffer_nels[1]; nelz = buffer_nels[2];
	numCptx = buffer_nels[0] + DEGREE; numCpty = buffer_nels[1] + DEGREE; numCptz = buffer_nels[2] + DEGREE;

	// Get all weights(same size as the controlPts) from binary file.
	read_float("../../../Asset/weights.bin", mesh_cp_weights);

	// Get xyz knots from binary file.
	std::vector<float> buffer_knots;
	read_float("../../../Asset/knots.bin", buffer_knots);
	int rowLength = buffer_knots.size() / 3;
	
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

	std::vector<float> buffer_rho;
	read_float("../../../Asset/rho.bin", buffer_rho);
	for (int i = 0; i < nelx; i++)
	{
		std::vector<std::vector<float>> face;
		for (int j = 0; j < nelz; j++)
		{
			std::vector<float> line;
			for (int k = 0; k < nely; k++)
			{
				line.push_back(buffer_rho[i * nelz * nely + j * nely + k]);
			}
			face.push_back(line);
		}
		mesh_rho.push_back(face);
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
	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/rendersurfacemy.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/rendersurfacemy.frag", &rendersurface)) {
		MYERROR("Could not load surface renderer shader");
		return false;
	}

	if (!GLCreateComputeProgramFromFile("../../../Asset/Shaders/GLSL/tessellatesurfacemyRE.comp", &tessellatesurfacemy)) {
		MYERROR("Could not load compute shader");
		return false;
	}
	
	screenquad = new OpenGLScreenQuad();

	// tessellate for the first time
	Tessellate();

	// FIXME: Modify the code of camera (maybe with AABB of the NURBS mesh)
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
	delete tessellatesurfacemy;
	delete screenquad;

	OpenGLContentManager().Release();
}

void Tessellate()
{
	for (auto& surface : surfacegroup)
	{
		delete surface;
	}
	surfacegroup.clear();

	OpenGLVertexElement decl[] = {
		{ 0, 0, GLDECLTYPE_FLOAT4, GLDECLUSAGE_POSITION, 0 },
		{ 0, 16, GLDECLTYPE_FLOAT4, GLDECLUSAGE_NORMAL, 0 },
		{ 0, 32, GLDECLTYPE_FLOAT4, GLDECLUSAGE_COLOR, 0},
		{ 0xff, 0, 0, 0, 0 }
	};

	for(int i = 0; i < 3; i++)
	{
		NURBSLayerData& layerData = mesh_layers[i];
		auto& cptsIndex = layerData.cptsIndex;
		auto& weights = layerData.weights;
		auto& knotU = layerData.knotU;
		auto& knotV = layerData.knotV;
		auto& knotW = layerData.knotW;
		auto& rho = layerData.LayerRho;

		int numCptU = cptsIndex[0].size();
		int numCptV = cptsIndex[0][0].size();
		int numCptW = cptsIndex.size();

		int nelU = numCptU - DEGREE, nelV = numCptV - DEGREE, nelW = numCptW - DEGREE;

		OpenGLMesh* surface = nullptr;

		// create surface
		// FIXME: How to get MaxSurfaceVertices and Indices?
		// FIXME: Hard-codes here make fault.
		if (!GLCreateMesh(MaxSurfaceVertices, MaxSurfaceIndices, GLMESH_32BIT, decl, &surface)) {
			MYERROR("Could not create surface");
			return;
		}

		// update surface cvs and weights (STL 2D vector will cause fault)
		Math::Vector4* surfacecvs = new Math::Vector4[numCptW * numCptU * numCptV];
		float* surfacewts = new float[numCptW * numCptU * numCptV];
		float* surfacerho = new float[nelW * nelU * nelV];
		uint32_t index;

		for(int s = 0; s < numCptW; s++) {
			for (int m = 0; m < numCptU; m++) {
				for (int n = 0; n < numCptV; n++) {
					index = s * numCptU * numCptV + m * numCptV + n;
					surfacecvs[index][0] = mesh_cp_vertices[cptsIndex[s][m][n] - 1].x;
					surfacecvs[index][1] = mesh_cp_vertices[cptsIndex[s][m][n] - 1].y;
					surfacecvs[index][2] = mesh_cp_vertices[cptsIndex[s][m][n] - 1].z;
					// std::cout << mesh_cp_vertices[cptsIndex[s][m][n] - 1].x << mesh_cp_vertices[cptsIndex[s][m][n] - 1].y << mesh_cp_vertices[cptsIndex[s][m][n] - 1].z << std::endl;
					surfacewts[index] = weights[s][m][n];
				}
			}
		}

		for(int s = 0; s < nelW; s++) {
			for (int m = 0; m < nelU; m++) {
				for (int n = 0; n < nelV; n++) {
					index = s * nelU * nelV + m * nelV + n;
					surfacerho[index] = rho[s][m][n];
				}
			}
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surface->GetVertexBuffer());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, surface->GetIndexBuffer());
		//-------------------------------------TEST PASS------------------------------------------

		glGenBuffers(1, &cptsBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, cptsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numCptU * numCptV * numCptW * sizeof(Math::Vector4), surfacecvs, GL_DYNAMIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, cptsBuffer);

		glGenBuffers(1, &wtsBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, wtsBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numCptU * numCptV * numCptW * sizeof(float), surfacewts, GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, wtsBuffer);

		glGenBuffers(1, &rhoBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, rhoBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nelU * nelV * nelW * sizeof(float), surfacerho, GL_STATIC_READ);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, rhoBuffer);
		//-------------------------------------TEST PASS------------------------------------------

		tessellatesurfacemy->SetInt("numVtsBetweenKnotsU", numSegBetweenKnotsU + 1);
		tessellatesurfacemy->SetInt("numVtsBetweenKnotsV", numSegBetweenKnotsV + 1);
		tessellatesurfacemy->SetInt("numControlPointsU", numCptU);
		tessellatesurfacemy->SetInt("numControlPointsV", numCptV);
		tessellatesurfacemy->SetInt("numControlPointsW", numCptW);
		tessellatesurfacemy->SetInt("degreeU", DEGREE);
		tessellatesurfacemy->SetInt("degreeV", DEGREE);
		tessellatesurfacemy->SetInt("degreeW", DEGREE);
		tessellatesurfacemy->SetFloatArray("knotsU", &knotU[0], numCptU + DEGREE + 1);
		tessellatesurfacemy->SetFloatArray("knotsV", &knotV[0], numCptV + DEGREE + 1);
		tessellatesurfacemy->SetFloatArray("knotsW", &knotW[0], numCptW + DEGREE + 1);

		int surfaceNum = nelW * 2;
		tessellatesurfacemy->Begin();
		{
			glDispatchCompute(nelU, nelV, nelW);
		}
		tessellatesurfacemy->End();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);

		int numSegmentsU = numSegBetweenKnotsU * nelU;
		int numSegmentsV = numSegBetweenKnotsV * nelV;

		surface->GetAttributeTable()->IndexCount = surfaceNum * numSegmentsU * numSegmentsV * 6;

		delete[] surfacecvs;

		surfacegroup.push_back(surface);
	}

}


void KeyUp(KeyCode key)
{
	switch (key) {

	case KeyCodeW:
		wireframe = !wireframe;
		break;

	case KeyCodeA:
		numSegBetweenKnotsU = Math::Min<int>(numSegBetweenKnotsU + 5, 20);
		numSegBetweenKnotsV = Math::Min<int>(numSegBetweenKnotsV + 5, 20);
		Tessellate();
		break;

	case KeyCodeD:
		numSegBetweenKnotsU = Math::Max<int>(numSegBetweenKnotsU - 5, 5);
		numSegBetweenKnotsV = Math::Max<int>(numSegBetweenKnotsV - 5, 5);
		Tessellate();
		break;

	case KeyCodeQ:
		mesh_cp_vertices[10].z -= 0.2f;
		Tessellate();
		break;

	case KeyCodeE:
		mesh_cp_vertices[10].z += 0.2f;
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
		for (int i = 0; i < 3; i++)
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

	// query limitations
	// -----------------
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;

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
