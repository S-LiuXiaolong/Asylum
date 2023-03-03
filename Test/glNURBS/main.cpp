#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

#define NOMINMAX
#include <iostream>

#include "application.h"
#include "gl4ext.h"
// #include "geometryutils.h"
// #include "basiccamera.h"
#include <GL/GL.h>
#include <GL/Glu.h>

#define TITLE				"glNURBS"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }

// sample variables
Application* app = nullptr;

GLfloat ctlpoints[4][4][3];

GLUnurbsObj *theNurb; // 指向一个NURBS曲面对象的指针

void init_surface(void)
{
	int u, v;
	for (u = 0; u < 4; u++) {
		for (v = 0; v < 4; v++) {
			ctlpoints[u][v][0] = 2.0 * ((GLfloat)u - 1.5);
			ctlpoints[u][v][1] = 2.0 * ((GLfloat)v - 1.5);

			if ((u == 1 || u == 2) && (v == 1 || v == 2))
				ctlpoints[u][v][2] = 3.0;
			else
				ctlpoints[u][v][2] = -3.0;
		}
	}
}

bool InitScene()
{
	uint32_t screenwidth = app->GetClientWidth();
	uint32_t screenheight = app->GetClientHeight();

	GLfloat mat_diffuse[] = { 0.6, 0.6, 0.6, 1.0 };
	GLfloat mat_specular[] = { 0.9, 0.9, 0.9, 1.0 };
	GLfloat mat_shininess[] = { 128.0 };

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	init_surface();

	theNurb = gluNewNurbsRenderer();
	gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 50.0);
	gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);

    return true;
}

void UninitScene()
{
	OpenGLContentManager().Release();
}

void Update(float delta)
{

}

void Render(float alpha, float elapsedtime)
{
	GLfloat knots[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 };
	GLfloat edgePt[5][2] = /* counter clockwise */
	{ {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0},
	 {0.0, 0.0} };
	GLfloat curvePt[4][2] = /* clockwise */
	{ {0.25, 0.5}, {0.25, 0.75}, {0.75, 0.75}, {0.75, 0.5} };
	GLfloat curveKnots[8] =
	{ 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0 };
	GLfloat pwlPt[4][2] = /* clockwise */
	{ {0.75, 0.5}, {0.5, 0.25}, {0.25, 0.5} };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	gluBeginSurface(theNurb);
	gluNurbsSurface(theNurb,
		8, knots,
		8, knots,
		4 * 3,
		3,
		&ctlpoints[0][0][0],
		4, 4,
		GL_MAP2_VERTEX_3);
	gluBeginTrim(theNurb);
	gluPwlCurve(theNurb, 5, &edgePt[0][0], 2,
		GLU_MAP1_TRIM_2);
	gluEndTrim(theNurb);
	gluBeginTrim(theNurb);
	gluNurbsCurve(theNurb, 8, curveKnots, 2,
		&curvePt[0][0], 4, GLU_MAP1_TRIM_2);
	gluPwlCurve(theNurb, 3, &pwlPt[0][0], 2,
		GLU_MAP1_TRIM_2);
	gluEndTrim(theNurb);
	gluEndSurface(theNurb);

	glFlush();
}

void KeyUp(KeyCode key)
{

}

void MouseMove(int32_t x, int32_t y, int16_t dx, int16_t dy)
{

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