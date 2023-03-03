#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#define NOMINMAX
#include <iostream>

#include "application.h"
#include "gl4ext.h"
#include "geometryutils.h"
#include "basiccamera.h"
#include <GL/Glu.h>

#define TITLE				"glNURBS"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// sample variables
Application* app = nullptr;

OpenGLMesh* mesh = nullptr;

OpenGLEffect* blinnphong = nullptr;

OpenGLScreenQuad* screenquad = nullptr;

BasicCamera camera;
BasicCamera light;

bool use_debug = false;

typedef GeometryUtils::CommonVertex CommonVertex;

GLUnurbsObj *theNurb; // 指向一个NURBS曲面对象的指针

bool InitScene()
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

    //背景色
    glClearColor(0.0, 0.0, 0.0, 1.0);
//     //代码开关3：设置材质与光源
//     GLfloat ambient[] = { 0.4, 0.6, 0.2, 1.0 };
//     GLfloat position[] = { 1.0, 1.0, 3.0, 1.0 };
//     GLfloat mat_diffuse[] = { 0.8, 0.6, 0.3, 1.0 };
//     GLfloat mat_specular[] = { 0.8, 0.6, 0.3, 1.0 };
//     GLfloat mat_shininess[] = { 45.0 };
// 
//     glEnable(GL_LIGHTING);
//     glEnable(GL_LIGHT0);
//     glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
//     glLightfv(GL_LIGHT0, GL_POSITION, position);
//     glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
//     glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
//     glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    //允许深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //代码开关4：取消下面两行代码，查看曲面显示效果差异
    //打开自动法矢量开关
    glEnable(GL_AUTO_NORMAL);
    //允许正则化法矢量
    // glEnable(GL_NORMALIZE);
    theNurb = gluNewNurbsRenderer(); // 创建一个NURBS曲面对象  
    //修改NURBS曲面对象的属性——glu库函数
    //采样sampling容错torerance
    gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 5.0);
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);

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
	camera.SetDistance(2.0f);
	camera.SetPosition(0, 0, 0);
	// camera.SetOrientation(Math::DegreesToRadians(135), 0.45f, 0);
	camera.SetOrientation(0, 0, 0);
	camera.SetPitchLimits(-Math::HALF_PI, Math::HALF_PI);

    return true;
}

void UninitScene()
{
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

		GLfloat ctrlpoints[6][5][3] = {
		{{-3,0,0}, {-1,1,0}, {0,0,0}, {1,-1,0}, {3,0,0}},
		{{-3,0,-1},{-1,1,-1},{0,0,-1},{1,-1,-1},{3,0,-1}},
		{{-3,0,-3},{-1,1,-3},{0,0,-3},{1,1,-3},{3,0,-3}},
		{{-3,1,-3},{-1,1,-3},{0,0,-3},{1,-1,-3},{3,0,-3}},
		{{-3,0,-4},{-1,1,-4},{0,0,-4},{1,-1,-4},{3,0,-4}},
		{{-3,2,-5},{-1,1,-5},{0,0,-5},{1,-1,-5},{3,1,-5}} };

		//各控制点影响力参数设置
		GLfloat knots1[12] = { 0.0, 0.0, 0.0, 0.0,0.0,0.0,
			1.0, 1.0, 1.0, 1.0 ,1.0,1.0 }; // NURBS曲面的控制向量 
		GLfloat knots2[10] = { 0.0, 0.0, 0.0, 0.0,0.0,
			1.0, 1.0, 1.0, 1.0 ,1.0 }; // NURBS曲面的控制向量     

		gluBeginSurface(theNurb); // 开始曲面绘制 	
		gluNurbsSurface(theNurb, 12, knots1, 10, knots2, 5 * 3, 3, &ctrlpoints[0][0][0], 6, 5, GL_MAP2_VERTEX_3); // 定义曲面的数学模型，确定其形状  
		gluEndSurface(theNurb); // 结束曲面绘制 
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

int main(int argc, char** argv)
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