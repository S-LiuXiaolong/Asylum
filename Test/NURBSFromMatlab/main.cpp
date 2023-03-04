#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#define NOMINMAX
#include <iostream>
#include <fstream>

#include "application.h"
#include "gl4ext.h"
#include "geometryutils.h"
#include "basiccamera.h"

#define TITLE				"mesh"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

 // sample variables
Application* app = nullptr;

// OpenGLMesh* stlmesh = nullptr;
OpenGLMesh* mesh = nullptr;

OpenGLEffect* blinnphong = nullptr;

OpenGLScreenQuad* screenquad = nullptr;

BasicCamera camera;

BasicCamera light;

bool use_debug = false;

typedef GeometryUtils::CommonVertex CommonVertex;

struct vertex { GLfloat x, y, z; };

#define ORDER 2
#define RESU 2
#define RESV 2

// struct vertex mesh_vertices[patch_number * RESU * RESV];
// GLfloat mesh_colors[patch_number * RESU * RESV * 3];
// GLuint mesh_elements[patch_number * (RESU - 1) * (RESV - 1) * 2 * 3];

// GLfloat mesh_cp_colors[269 * 3];
// GLuint mesh_cp_elements[patch_number][ORDER + 1][ORDER + 1];

std::vector<vertex> mesh_cp_vertices;
// GLuint (*mesh_patches)[ORDER + 1][ORDER + 1];
std::vector<std::vector<std::vector<GLuint>>> mesh_patches;

// vertex* mesh_vertices;
// GLfloat* mesh_colors;
// GLuint* mesh_elements;
std::vector<vertex> mesh_vertices;
std::vector<GLfloat> mesh_colors;
std::vector<GLuint> mesh_elements;

GLfloat* mesh_cp_colors;
GLuint* mesh_cp_elements;

void read_controlPts(std::string strFile, std::vector<float>& buffer);
void read_elements(std::string strFile, std::vector<uint32_t>& buffer);
// TODO: read_density
void read_density(std::string strFile, std::vector<float>& buffer);
void build_patches(std::vector<uint32_t>& buffer_eles);
void build_control_points_k(int p, struct vertex control_points_k[][ORDER + 1]);
struct vertex compute_position(struct vertex control_points_k[][ORDER + 1], float u, float v);
float bernstein_polynomial(int i, int n, float u);
float binomial_coefficient(int i, int n);
int factorial(int n);

void build_mesh() {
    // TODO: hard code here
    std::vector<float> buffer_cpts;
    read_controlPts("../../../Asset/controlPts.bin", buffer_cpts);

	mesh_cp_vertices.resize(buffer_cpts.size() / 3);

    for (int i = 0; i < buffer_cpts.size() / 3; i++)
    {
        mesh_cp_vertices[i] = { buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2] };
        // printf("%f %f %f\n", buffer_cpts[i * 3], buffer_cpts[i * 3 + 1], buffer_cpts[i * 3 + 2]);
    }

    std::vector<uint32_t> buffer_eles;
    read_elements("../../../Asset/element.bin", buffer_eles);

    int patch_number = buffer_eles.size() / 27 * 6;
// 	std::vector<uint32_t> buffer_one_ele;
// 	for (int i = 0; i < 27; i++)
// 	{
// 		buffer_one_ele.push_back(buffer_eles[i]);
// 	}
// 	patch_number = 6;
// 	build_patches(buffer_one_ele);

	build_patches(buffer_eles);

// 	mesh_vertices = new vertex[patch_number * RESU * RESV];
// 	mesh_colors = new GLfloat[patch_number * RESU * RESV * 3];
// 	mesh_elements = new GLuint[patch_number * (RESU - 1) * (RESV - 1) * 2 * 3];
	mesh_vertices.resize(patch_number * RESU * RESV);
	mesh_colors.resize(patch_number * RESU * RESV * 3);
	mesh_elements.resize(patch_number * (RESU - 1) * (RESV - 1) * 2 * 3);

	// Vertices
	for (int p = 0; p < patch_number; p++) {
		struct vertex control_points_k[ORDER + 1][ORDER + 1];
		build_control_points_k(p, control_points_k);
		for (int ru = 0; ru <= RESU - 1; ru++) {
			float u = 1.0 * ru / (RESU - 1);
			for (int rv = 0; rv <= RESV - 1; rv++) {
				float v = 1.0 * rv / (RESV - 1);
				mesh_vertices[p * RESU * RESV + ru * RESV + rv] = compute_position(control_points_k, u, v);
// 				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 0] = 1.0 * p / patch_number;
// 				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 1] = 1.0 * p / patch_number;
// 				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 2] = 0.8;

				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 0] = 0.0f;
				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 1] = 0.0f;
				mesh_colors[p * RESU * RESV * 3 + ru * RESV * 3 + rv * 3 + 2] = 1.0f;
			}
		}
	}

	// Elements
	int n = 0;
	for (int p = 0; p < patch_number; p++)
		for (int ru = 0; ru < RESU - 1; ru++)
			for (int rv = 0; rv < RESV - 1; rv++) {
				// 1 square ABCD = 2 triangles ABC + CDA
				// ABC
				mesh_elements[n] = p * RESU * RESV + ru * RESV + rv; n++;
				mesh_elements[n] = p * RESU * RESV + ru * RESV + (rv + 1); n++;
				mesh_elements[n] = p * RESU * RESV + (ru + 1) * RESV + (rv + 1); n++;
				// CDA
				mesh_elements[n] = p * RESU * RESV + (ru + 1) * RESV + (rv + 1); n++;
				mesh_elements[n] = p * RESU * RESV + (ru + 1) * RESV + rv; n++;
				mesh_elements[n] = p * RESU * RESV + ru * RESV + rv; n++;
			}

// 	// Control points elements for debugging
// 	memset(mesh_cp_colors, 0, sizeof(mesh_cp_colors)); // black
// 	for (int p = 0; p < patch_number; p++)
// 		for (int i = 0; i < (ORDER + 1); i++)
// 			for (int j = 0; j < (ORDER + 1); j++)
// 				mesh_cp_elements[p][i][j] = mesh_patches[p][i][j] - 1;
}

void read_controlPts(std::string strFile, std::vector<float>& buffer)
{
    float temp;
    std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
        return;
	}

	// 获取文件大小
	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
    infile.seekg(0);

    printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

    while (infile.read((char*)&temp, sizeof(float)))
    {
        int readedBytes = infile.gcount(); //看刚才读了多少字节
        // printf("%f\n", temp);
        buffer.push_back(temp);
    }
}

void read_elements(std::string strFile, std::vector<uint32_t>& buffer)
{
    uint32_t temp;
	std::ifstream infile(strFile.c_str(), std::ifstream::binary);
	if (!infile.is_open())
	{
		printf("Read File:%s Error ... \n", strFile.c_str());
		return;
	}

	// 获取文件大小
	infile.seekg(0, std::ifstream::end);
	long size = infile.tellg();
	infile.seekg(0);

	printf("The file: [%s] has: %ld(byte) ..... \n", strFile.c_str(), size);

	while (infile.read((char*)&temp, sizeof(uint32_t)))
	{
		int readedBytes = infile.gcount(); //看刚才读了多少字节
		// printf("%f\n", temp);
		buffer.push_back(temp);
	}

}


void build_patches(std::vector<uint32_t>& buffer_eles)
{	
	for (int i = 0; i < buffer_eles.size() / 27; i++)
	{
		std::vector<GLuint> row_elements;
		for (int j = 0; j < 27; j++)
		{
			row_elements.push_back(buffer_eles[i * 27 + j]);
		}
		std::vector<std::vector<GLuint>> patch_down = { {row_elements[0],row_elements[1],row_elements[2]},{row_elements[3],row_elements[4],row_elements[5]},{row_elements[6],row_elements[7],row_elements[8]} };
		std::vector<std::vector<GLuint>> patch_up = { {row_elements[18],row_elements[19],row_elements[20]},{row_elements[21],row_elements[22],row_elements[23]},{row_elements[24],row_elements[25],row_elements[26]} };
		std::vector<std::vector<GLuint>> patch_left = { {row_elements[0],row_elements[1],row_elements[2]},{row_elements[9],row_elements[10],row_elements[11]},{row_elements[18],row_elements[19],row_elements[20]} };
		std::vector<std::vector<GLuint>> patch_right = { {row_elements[6],row_elements[7],row_elements[8]},{row_elements[15],row_elements[16],row_elements[17]},{row_elements[24],row_elements[25],row_elements[26]} };
		std::vector<std::vector<GLuint>> patch_front = { {row_elements[2],row_elements[5],row_elements[8]},{row_elements[11],row_elements[14],row_elements[17]},{row_elements[20],row_elements[23],row_elements[26]} };
		std::vector<std::vector<GLuint>> patch_back = { {row_elements[0],row_elements[3],row_elements[6]},{row_elements[9],row_elements[12],row_elements[15]},{row_elements[18],row_elements[21],row_elements[24]} };

		mesh_patches.push_back(patch_down);
		mesh_patches.push_back(patch_up);
		mesh_patches.push_back(patch_left);
		mesh_patches.push_back(patch_right);
		mesh_patches.push_back(patch_front);
		mesh_patches.push_back(patch_back);
	}
}

void build_control_points_k(int p, struct vertex control_points_k[][ORDER + 1]) {
	for (int i = 0; i <= ORDER; i++)
		for (int j = 0; j <= ORDER; j++)
			control_points_k[i][j] = mesh_cp_vertices[mesh_patches[p][i][j] - 1];
}

struct vertex compute_position(struct vertex control_points_k[][ORDER + 1], float u, float v) {
	struct vertex result = { 0.0, 0.0, 0.0 };
	for (int i = 0; i <= ORDER; i++) {
		float poly_i = bernstein_polynomial(i, ORDER, u);
		for (int j = 0; j <= ORDER; j++) {
			float poly_j = bernstein_polynomial(j, ORDER, v);
			result.x += poly_i * poly_j * control_points_k[i][j].x;
			result.y += poly_i * poly_j * control_points_k[i][j].y;
			result.z += poly_i * poly_j * control_points_k[i][j].z;
		}
	}
	return result;
}

float bernstein_polynomial(int i, int n, float u) {
	return binomial_coefficient(i, n) * powf(u, i) * powf(1 - u, n - i);
}

float binomial_coefficient(int i, int n) {
	assert(i >= 0); assert(n >= 0);
	return 1.0f * factorial(n) / (factorial(i) * factorial(n - i));
}
int factorial(int n) {
	assert(n >= 0);
	int result = 1;
	for (int i = n; i > 1; i--)
		result *= i;
	return result;
}

int InitScene()
{
	build_mesh();

	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);

// 	glEnable(GL_CULL_FACE);
// 	glCullFace(GL_BACK);
// 
// 	glEnable(GL_DEPTH_TEST);
// 	glDepthFunc(GL_LESS);

/////////////////////////////////////TODO/////////////////////////////////////////////

	// create mesh
	int vertexSize = mesh_vertices.size();
	int indexSize = mesh_elements.size();

	OpenGLVertexElement decl[] = {
		{ 0, 0, GLDECLTYPE_FLOAT3, GLDECLUSAGE_POSITION, 0 },
		{ 0, 12, GLDECLTYPE_FLOAT4, GLDECLUSAGE_COLOR, 0 },
		{ 0xff, 0, 0, 0, 0 }
	};

	if (!GLCreateMesh(vertexSize, indexSize, GLMESH_32BIT, decl, &mesh))
		return false;

	OpenGLAttributeRange table[] = {
		{ GLPT_TRIANGLELIST, 0, 0, indexSize, 0, vertexSize, true }
	};

	struct colorVertex
	{
		Math::Vector3 pos;
		Math::Vector4 color;
	};

	colorVertex* vdata = nullptr;
	int* idata = new int[indexSize];
	GLuint numsubsets = 1;

	// TODO: remember to subtract with 1 when putting index into indexbuffer
	// (index of INP node start with 1 and OpenGL indexbuffer start with 0)
	mesh->LockVertexBuffer(0, 0, GLLOCK_DISCARD, (void**)&vdata);
	mesh->LockIndexBuffer(0, 0, GLLOCK_DISCARD, (void**)&idata);
	{
		// vertex data
		{
			for (int i = 0; i < vertexSize; i++)
			{
				vdata[i].pos.x = mesh_vertices[i].x;
				vdata[i].pos.y = mesh_vertices[i].y;
				vdata[i].pos.z = mesh_vertices[i].z;

				vdata[i].color.x = mesh_colors[i * 3];
				vdata[i].color.y = mesh_colors[i * 3 + 1];
				vdata[i].color.z = mesh_colors[i * 3 + 2];
				vdata[i].color.w = 1.0f;
			}
		}

		// index data
		{
			for (int index = 0; index < indexSize; index++)
			{
				idata[index] = mesh_elements[index];
			}
		}
	}
	mesh->UnlockIndexBuffer();
	mesh->UnlockVertexBuffer();

	mesh->SetAttributeTable(table, numsubsets);

/////////////////////////////////////TODO/////////////////////////////////////////////

	if (!GLCreateEffectFromFile("../../../Asset/Shaders/GLSL/simpleblinn.vert", 0, 0, 0, "../../../Asset/Shaders/GLSL/simpleblinn.frag", &blinnphong)) {
		MYERROR("Could not load 'simpleblinn' effect");
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
	camera.SetDistance(5.0f);
	camera.SetPosition(0, 0, 0);
	// camera.SetOrientation(Math::DegreesToRadians(135), 0.45f, 0);
	camera.SetOrientation(0, 0, 0);
	camera.SetPitchLimits(-Math::HALF_PI, Math::HALF_PI);

	return true;
}

void UninitScene()
{
//	delete stlmesh;
	delete mesh;

	delete blinnphong;
	
	delete screenquad;

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
	Math::Vector3 lightpos = { 0, 1, 0 };
	Math::Color color = { 1, 1, 1, 1 };

	camera.Animate(alpha);

	camera.GetViewMatrix(view);
	camera.GetProjectionMatrix(proj);
	camera.GetEyePosition(eye);

	Math::MatrixMultiply(viewproj, view, proj);
	Math::MatrixIdentity(world);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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