#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#define _SECURE_SCL_DEPRECATE 0
#define _CRT_SECURE_NO_WARNINGS 0
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#define PI 3.1415926535897
#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else
#define DAQmx_Val_GroupByChannel 0

// OpenGL is pretty senstive so our header file glut.h has to be the first on our include files
#include <GL/glut.h> // MUST BE FIRST (not including MACROS. Those are fine first.)
#include <GL/GL.h>
#include <GL/GLU.h>
#include <GL/freeglut.h>
#include <time.h>       /* time_t, clock, CLOCKS_PER_SEC */
#include <math.h>       /* sqrt */
#include <iostream>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <NIDAQmx.h>
#include <time.h>
#include <windows.h>
#include "wglext.h"
//#include <string.h>
//#include <GL/glui.h>
GLuint texture[1];

double angle = 0;
bool clear;
int yp;
int centering = 0;
float viewingAngle = 10;
float delay = 120;
bool drifting = 0;
float driftVel = 10;
bool closedLoop = 0;
bool horizontal = 0;
bool spinning = 0;
bool vertical = 1;

typedef struct
{

	int X;

	int Y;

	int Z;



	double U;

	double V;
}VERTICES;

//const double PI = 3.1415926535897;

//const int space = 10;
const int space = 10;


//const int VertexCount = (90 / space) * (360 / space) * 4;
const int VertexCount = (360 / space) * (360 / space) * 4;


VERTICES VERTEX[VertexCount];

GLuint LoadTextureRAW(const char * filename);

bool WGLExtensionSupported(const char *extension_name)
{
	// this is pointer to function which returns pointer to string with list of all wgl extensions
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

	// determine pointer to wglGetExtensionsStringEXT function
	_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

	if (_wglGetExtensionsStringEXT == 0 ||
		(_wglGetExtensionsStringEXT == (void*)0x1) || (_wglGetExtensionsStringEXT == (void*)0x2) || (_wglGetExtensionsStringEXT == (void*)0x3) ||
		(_wglGetExtensionsStringEXT == (void*)-1))
	{
		int a = 23;
		HMODULE module = LoadLibraryA("opengl32.dll");
		_wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)GetProcAddress(module, "wglGetExtensionsStringEXT");
	}

	if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
	{
		// string was not found
		printf("\nExtension not supported");
		return false;
	}

	// extension is supported
	printf("\nExtension supported");
	return true;
}

void DisplaySphere(double R, GLuint texture) {

	int b;



	//glScalef(0.0125 * R, 0.0125 * R, 0.0125 * R);


	if (horizontal) {
		glRotatef(90, 0, 1, 0);
	}
	else if (vertical) {
		glRotatef(90, 1, 0, 0);
	}


	glBindTexture(GL_TEXTURE_2D, texture);



	glBegin(GL_TRIANGLE_STRIP);

	for (b = 0; b <= VertexCount; b++) {

		if (fmod(b, 1) == 0) {
			glTexCoord2f(VERTEX[b].U, VERTEX[b].V);

			glVertex3f(VERTEX[b].X, VERTEX[b].Y, -VERTEX[b].Z);
		}

	}



	for (b = 0; b <= VertexCount; b++) {

		if (fmod(b, 1) == 0) {
			glTexCoord2f(VERTEX[b].U, -VERTEX[b].V);

			glVertex3f(VERTEX[b].X, VERTEX[b].Y, VERTEX[b].Z);
		}

	}

	glEnd();
}
void CreateSphere(double R, double H, double K, double Z) {

	int n;

	double a;

	double b;



	n = 0;



	//for (b = 0; b <= 90 - space; b += space) {
	for (b = 0; b <= 360 - space; b += space) {


		for (a = 0; a <= 360 - space; a += space) {

			if (fmod(b,2 * viewingAngle) < viewingAngle + yp) {

				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = R * sin((a) / 180 * PI) * cos((b) / 180 * PI) - H;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) + K;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a) / 180 * PI) - Z;


				VERTEX[n].V = (2 * b) / 360;

				VERTEX[n].U = (a) / 360;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI
				VERTEX[n].X = R * sin((a) / 180 * PI) * cos((b + space) / 180 * PI


				) - H;

				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b + space) / 180 * PI
				VERTEX[n].Y = R * sin((a) / 180 * PI) * sin((b + space) / 180 * PI


				) + K;

				//VERTEX[n].Z = R * cos((b + space) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a + space) / 180 * PI) - Z;


				VERTEX[n].V = (2 * (b + space)) / 360;

				VERTEX[n].U = (a) / 360;

				n++;



				//VERTEX[n].X = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI
				VERTEX[n].X = R * sin((a + space) / 180 * PI) * cos((b) / 180 * PI

				) - H;

				//VERTEX[n].Y = R * cos((a + space) / 180 * PI) * sin((b) / 180 * PI
				VERTEX[n].Y = R * sin((a + space) / 180 * PI) * sin((b) / 180 * PI

				) + K;

				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a) / 180 * PI) - Z;


				VERTEX[n].V = (2 * b) / 360;

				VERTEX[n].U = (a + space) / 360;

				n++;



				//VERTEX[n].X = R * sin((a + space) / 180 * PI) * sin((b + space) /
				VERTEX[n].X = R * sin((a + space) / 180 * PI) * cos((b + space) /

					180 * PI) - H;

				//VERTEX[n].Y = R * cos((a + space) / 180 * PI) * sin((b + space) /
				VERTEX[n].Y = R * sin((a + space) / 180 * PI) * sin((b + space) /

					180 * PI) + K;

				//VERTEX[n].Z = R * cos((b + space) / 180 * PI) - Z;
				VERTEX[n].Z = R * cos((a + space) / 180 * PI) - Z;


				VERTEX[n].V = (2 * (b + space)) / 360;

				VERTEX[n].U = (a + space) / 360;

				n++;
			}


		}

	}
}

void display(void) {

	glClearDepth(1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();



	//glTranslatef(0, 0, -10);

	if (drifting) {
		glRotatef(driftVel * angle, horizontal, vertical, spinning);
	}
	else {
		glRotatef((180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0, horizontal, vertical, spinning);
	}

	if (!clear) {
		DisplaySphere(5, texture[0]);
	}



	glutSwapBuffers();

	angle++;
}

/*
* Callback function to retrieve key value movement
* @param key - ASCII value for key input
* @param x -  n/a
* @param y - n/a
*/
void letter_pressed(unsigned char key, int x, int y) {
	float degree;
	float frequency;
	float driftDeg; //drift in degrees/sec
	switch (key) {
	case 98: //b
		if (clear) {
			yp = 0;
			clear = false;
		}
		else {
			yp = -800;
			clear = true;
		}
		glutPostRedisplay();
		break;
	case 66: //B
		if (clear) {
			yp = 0;
			clear = false;
		}
		else {
			yp = -800;
			clear = true;
		}
		glutPostRedisplay();
		break;
	case 99: //c
		centering = 1;
		break;
	case 67: //C
		centering = 1;
		break;
	case 27: // ESC to exit fullscreen
		exit(0);
		break;

		//Not implemented yet
		/*
	case 114: //r
		
		printf("We are entering our switch case\n");
		glPushMatrix();
		glTranslatef(200, 300, 0);
		glRotatef(90, 0, 0, 1);
		glBegin(GL_QUADS);
		{
			glVertex2f(-num / 2, -num2 / 2);
			glVertex2f(-num2 / 2, -num2 / 2);
			glVertex2f(num2 / 2, num3 / 2);
			glVertex2f(num / 2, num3 / 2);
		}
		glEnd();
		glPopMatrix();
		break;
		*/
		/*
	case 45: //- will shrink bar
		if (barwidthIt > 0) {
			barwidthIt--;
			barwidth = barwidthArr[barwidthIt];
			if (horizontal) {
				//barwidth *= 1.763313609;
			}
		}
		glutPostRedisplay();
		break;
	case 61: //= will enlarge bar
		if (barwidthIt < 2) {
			barwidthIt++;
			barwidth = barwidthArr[barwidthIt];
			if (horizontal) {
				//barwidth *= 1.763313609;
			}
		}
		glutPostRedisplay();
		break;
	case 49: //1 will make stimulus single bar
		bars[0] = 0;
		bars[1] = 0;
		bars[2] = 0;
		bars[3] = 0;
		bars[4] = 0;
		bars[5] = 0;
		bars[6] = 0;
		bars[7] = 0;
		bars[8] = 0;
		bars[9] = 0;
		bars[10] = 0;
		bars[11] = 0;
		bars[12] = 0;
		bars[13] = 0;
		bars[14] = 0;
		bars[15] = 0;
		bars[16] = 0;
		bars[17] = 0;
		glutPostRedisplay();
		break;
	case 50: //2 will make stimulus 5-bar grate
		bars[0] = -32;
		bars[1] = -28;
		bars[2] = -24;
		bars[3] = -20;
		bars[4] = -16;
		bars[5] = -12;
		bars[6] = -8;
		bars[7] = -4;
		bars[8] = 0;
		bars[9] = 4;
		bars[10] = 8;
		bars[11] = 12;
		bars[12] = 16;
		bars[13] = 20;
		bars[14] = 24;
		bars[15] = 28;
		bars[16] = 32;
		bars[17] = 36;
		glutPostRedisplay();
		break;
		*/
	case 86: //V will request viewing angle
		printf("\nInput viewing angle in degrees: ");
		scanf("%f", &degree);
		viewingAngle = degree;
		//barwidth = (float) 0.307975 * tanf(degree * PI / (2.0 * 180.0)) * 1342.281879;
		printf("%f", viewingAngle);
		CreateSphere(70, 0, 0, 0);
		glutPostRedisplay();
		break;
	case 118: //v will request viewing angle
		printf("\nInput viewing angle in degrees: ");
		scanf("%f", &degree);
		viewingAngle = degree;
		//barwidth = (float) 0.307975 * tanf(degree * PI / (2.0 * 180.0)) * 1342.281879;
		printf("%f", viewingAngle);
		CreateSphere(70, 0, 0, 0);
		glutPostRedisplay();
		break;
	case 70: //F will request frequency
		printf("\nInput oscillation frequency in Hz: ");
		scanf("%f", &frequency);
		delay = 120.0 / frequency;
		drifting = 0;
		glutPostRedisplay();
		break;
	case 102: //f will request frequency
		printf("\nInput oscillation frequency in Hz: ");
		scanf("%f", &frequency);
		delay = 120.0 / frequency;
		drifting = 0;
		glutPostRedisplay();
		break;
	case 68: //D will request driftDeg
		printf("\nInput drift velocity in degrees/sec: ");
		scanf("%f", &driftDeg);
		//driftVel = 2.0*PI*0.307975*(driftDeg / 360.0)*1342.281879*(1.0 / 120.0);
		driftVel = driftDeg / 120.0;
		drifting = 1;
		glutPostRedisplay();
		break;
	case 100: //d will request driftDeg
		printf("\nInput drift velocity in degrees/sec: ");
		scanf("%f", &driftDeg);
		//driftVel = 2.0*PI*0.307975*(driftDeg / 360.0)*1342.281879*(1.0 / 120.0);
		driftVel = driftDeg / 120.0;
		drifting = 1;
		glutPostRedisplay();
		break;
	case 79: //O will make open-loop
		printf("\nNow in open-loop.");
		closedLoop = 0;
		centering = 1;
		glutPostRedisplay();
		break;
	case 111: //o will make open-loop
		printf("\nNow in open-loop.");
		closedLoop = 0;
		centering = 1;
		glutPostRedisplay();
		break;
	case 80: //P will make closed-loop
		printf("\nNow in closed-loop.");
		closedLoop = 1;
		glutPostRedisplay();
		break;
	case 112: //p will make closed-loop
		printf("\nNow in closed-loop.");
		closedLoop = 1;
		glutPostRedisplay();
		break;
	case 72: //H will make horizontal bars
		printf("\nBars are now horizontal.");
		horizontal = 1;
		spinning = 0;
		vertical = 0;		//barwidth = barwidthArr[barwidthIt] * 1.763313609;
		glutPostRedisplay();
		break;
	case 104: //h will make horizontal bars
		printf("\nBars are now horizontal.");
		horizontal = 1;
		spinning = 0;
		vertical = 0;		//barwidth = barwidthArr[barwidthIt] * 1.763313609;
		glutPostRedisplay();
		break;
	case 76: //L will make vertical bars
		printf("\nBars are now vertical.");
		horizontal = 0;
		spinning = 0;
		vertical = 1;
		//barwidth = barwidthArr[barwidthIt];
		glutPostRedisplay();
		break;
	case 108: //l will make vertical bars
		printf("\nBars are now vertical.");
		horizontal = 0;
		spinning = 0;
		vertical = 1;
		//barwidth = barwidthArr[barwidthIt];
		glutPostRedisplay();
		break;
	case 83: //S will make spinny stuff
		horizontal = 0;
		spinning = 1;
		vertical = 0;
		printf("\nSpinning.");
		break;
	case 115: //s will make spinny stuff
		horizontal = 0;
		spinning = 1;
		vertical = 0;
		printf("\nSpinning.");
		break;
	}
}

void init(void) {

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);

	glDepthFunc(GL_LEQUAL);

	glCullFace(GL_BACK);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	//texture[0] = LoadTextureRAW(“earth.raw”);

	CreateSphere(70, 0, 0, 0);
}
void reshape(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	glutInitWindowSize(500, 500);

	glutInitWindowPosition(100, 100);

	glutCreateWindow("A basic OpenGL Window");

	init();

	PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		// this is another function from WGL_EXT_swap_control extension
		wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglSwapIntervalEXT(-1);
	}

	glutFullScreen(); //This makes shit fullscreen

	glutDisplayFunc(display);

	glutIdleFunc(display);

	glutReshapeFunc(reshape);

	glutKeyboardFunc(letter_pressed);

	glutMainLoop();

	return 0;
}

GLuint LoadTextureRAW(const char * filename)
{

	GLuint texture;

	int width, height;

	unsigned char * data;

	FILE * file;



	file = fopen(filename, "rb");

	if (file == NULL) return 0;



	width = 1024;

	height = 512;

	data = (unsigned char *)malloc(width * height * 3);



	fread(data, width * height * 3, 1, file);

	fclose(file);



	glGenTextures(1, &texture);



	glBindTexture(GL_TEXTURE_2D, texture);



	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,

		GL_MODULATE);



	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,

		GL_LINEAR_MIPMAP_NEAREST);



	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,

		GL_LINEAR);



	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,

		GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,

		GL_REPEAT);



	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,

		GL_RGB, GL_UNSIGNED_BYTE, data);



	free(data);



	return texture;


}