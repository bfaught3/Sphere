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

#ifndef M_PI
#define M_PI            (3.14159265358979f)
#endif
#define DTOR            (M_PI/180.0)
#define RTOD            (180.0/M_PI)

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
//#include <mutex>
//#include <thread>
//#include <string.h>
//#include <GL/glui.h>
GLuint texture[1];

double angle = 0;
bool clear;
int yp;
int centering = 0;
float viewingAngle = 10;
//float delay = 1200; //0.1 Hz
float delay = 10000.0; //0.1 Hz
bool drifting = 0;
float driftVel = 10;
bool closedLoop = 0;
bool horizontal = 0;
bool spinning = 0;
bool vertical = 1;
const float R = 584.6207004;
int single = 360;
//const float R = 70;
float weight = 1;
int increment = 0;
float CLangle = 0;
float angVel = 0;
float OLangle = 0;
const float threshold = 0.0025;
int window1; //Main window
int window2;
int window3;
float oscillationAmp = 180.0; // This is the amplitude of the oscillation in degrees.
bool written = 0;


// for NIDAQ data handling
int32       error = 0;
TaskHandle  taskHandle = 0;
int32       read;
float64     data[1400700];
float64		currentData[1400700];
float64		currai0[200100];
float64		currai1[200100];
float64		currai2[200100];
float64		currai3[200100];
float64		currai4[200100];
float64		currai5[200100];
int64		currai6[200100];
float64		currxp[200100];
float64		currxpcl[200100];	// closed loop
int32		queueit = 0;
float64		bias0, bias1, bias2, bias3, bias4, bias5;

char        errBuff[2048] = { '\0' };


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

GLfloat vertices[3 * VertexCount];
GLfloat norm[3 * VertexCount];
GLfloat col[3 * VertexCount];
GLfloat arr[9 * VertexCount];
GLfloat arr2[94608]; // (360/5) * (360/5) * 2 * 9???

static unsigned int fps_start = 0;
static unsigned int fps_frames = 0;

static unsigned int t_0 = 0.0;

static unsigned int freq_start = 0;
bool freq_measured = 0;
//bool isNeg = 0; // True if OLangle is negative.

PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;

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


/*
* Tests to see if the file exists.
**/
inline bool exists_test3(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

/*
* Allows for the user to printout data obtain from NIDAQ channels.
* Currently will start outputting to file after trigger has been pressed.
**/
void writeToFile() {
	FILE * fileP = NULL;

	time_t rawtime;
	struct tm *info;
	char buffer[80];

	time(&rawtime);
	info = localtime(&rawtime);
	strftime(buffer, 80, "%m%d%Y", info);

	char filename[80];
	sprintf(filename, "./Data/Moth_FT_%s_%03d.txt", buffer, increment);
	bool exists = exists_test3(filename);

	while (exists) {
		increment++;
		sprintf(filename, "./Data/Moth_FT_%s_%03d.txt", buffer, increment);
		//printf("\n%s", filename);
		exists = exists_test3(filename);
	}

	fileP = fopen(filename, "w");
	printf("\n%s", filename);

	if (fileP != NULL) {
		for (int i = 0; i < 200100; i++) {
			int helperQueue = queueit + i;
			if (helperQueue >= 200100) {
				helperQueue -= 200100;
			}
			fprintf(fileP, "%f, %f, %f, %f, %f, %f, %f, %f, %lld\n", currxp[helperQueue], currxpcl[helperQueue], currai0[helperQueue], currai1[helperQueue], currai2[helperQueue], currai3[helperQueue], currai4[helperQueue], currai5[helperQueue], currai6[helperQueue]);
		}
		fclose(fileP);
		increment++;
		char gs[80];
		char oc[80];
		char dh[80]; //Degrees/s vs Hz
		float dhv;
		char rpy[80]; //Roll, pitch, or yaw
		if (spinning) {
			sprintf(rpy, "Roll");
		}
		else if (horizontal) {
			sprintf(rpy, "Pitch");
		}
		else {
			sprintf(rpy, "Yaw");
		}
		if (single) {
			sprintf(gs, "Single Bar");
		}
		else {
			sprintf(gs, "Grating");
		}
		if (closedLoop) {
			sprintf(oc, "Closed");
		}
		else {
			sprintf(oc, "Open");
		}
		if (drifting) {
			sprintf(dh, "degrees per second");
			dhv = driftVel * 1000.0;
			//dhv = driftVel * 60.0;
		}
		else {
			sprintf(dh, "Hz");
			dhv = 1000.0 / delay;
			//dhv = 60.0 / delay;
		}
		//printf("\nFreq = %f Hz, %s, %s, %f deg", 120.0 / delay, gs, oc, viewingAngle);
		//printf("\nFreq = %f Hz, %s, %s, %f deg", 1000.0 / delay, gs, oc, viewingAngle);
		printf("\n%s stimulus, Freq = %f %s, %s, %s, %f deg", rpy, dhv, dh, gs, oc, viewingAngle);
	}
	else {
		printf("\nOur file cannot be written to");
	}

}

/*
*  Calculates the force along the x-axis, averaged over the most recent samples. The units are in Newtons.
**/
float64 calcFeedback() {
	float64 avgai0 = 0;
	for (int i = 0; i < read; i++) {
		int32 j = queueit - read + i;
		if (spinning) {
			if (j < 0) {
				avgai0 += currai4[j + 200100];
			}
			else {
				avgai0 += currai4[j];
			}
		}
		else if (horizontal) { //If horizontal, read from Tx
			if (j < 0) {
				avgai0 += currai3[j + 200100];
			}
			else {
				avgai0 += currai3[j];
			}
		}
		else {
			if (j < 0) { //If vertical, read from Tz
				avgai0 += currai5[j + 200100];
			}
			else {
				avgai0 += currai5[j];
			}
		}
	}
	avgai0 = avgai0 / read;

	return -avgai0;		// This is negative because the force transducer is backwards.
}

float64 biasing(float64 *readArray) {
	float64 avgai = 0;
	int ignore = 0;
	for (int i = ignore; i < read; i++) {
		int32 j = queueit - read + i;
		if (j < 0) {
			avgai += readArray[j + 200100];
		}
		else {
			avgai += readArray[j];
		}
	}
	avgai = avgai / (read - ignore);
	return avgai;
}

/*
* Allows for the conversion of voltages into force and torque.
**/
float64 * matrixMult(float64 ai0, float64 ai1, float64 ai2, float64 ai3, float64 ai4, float64 ai5) {

	float64 Fx = ((-0.000352378) * ai0) + (0.020472451 * ai1) + ((-0.02633045) * ai2) + ((-0.688977299) * ai3) + (0.000378075 * ai4) + (0.710008955 * ai5);
	float64 Fy = ((-0.019191418) * ai0) + (0.839003543 * ai1) + ((-0.017177775) * ai2) + ((-0.37643613) * ai3) + (0.004482987 * ai4) + ((-0.434163392) * ai5);
	float64 Fz = (0.830046806 * ai0) + (0.004569748 * ai1) + (0.833562339 * ai2) + (0.021075403 * ai3) + (0.802936538 * ai4) + ((-0.001350335) * ai5);
	float64 Tx = ((-0.316303442) * ai0) + (5.061378026 * ai1) + (4.614179159 * ai2) + ((-2.150699522) * ai3) + ((-4.341889297) * ai4) + ((-2.630773662) * ai5);
	float64 Ty = ((-5.320003676) * ai0) + ((-0.156640061) * ai1) + (2.796170871 * ai2) + (4.206523866 * ai3) + (2.780562472 * ai4) + ((-4.252850011) * ai5);
	float64 Tz = ((-0.056240509) * ai0) + (3.091367987 * ai1) + (0.122101875 * ai2) + (2.941467741 * ai3) + (0.005876647 * ai4) + (3.094672928 * ai5);

	Tx = Tx + 94.5 * Fy;
	Ty = Ty - 94.5 * Fx;

	float64 transformedData[6] = { Fx, Fy, Fz, Tx, Ty, Tz };
	return transformedData;
}

void DisplaySphere(double R, GLuint texture) {

	int b;



	//glScalef(0.0125 * R, 0.0125 * R, 0.0125 * R);


	if (horizontal) {
		//glRotatef(90, 0, 1, 0);
		switch ((int)R) {
		case 2:
			//glRotatef(90, 1, 0, 0);
			break;
		case 3:
			//glRotatef(-90, 1, 0, 0);
			break;
		default:
			break;
		}
		//glRotatef(90, 1, 0, 0);
		glRotatef(90, 0, 1, 0);
	}
	else if (vertical) {
		//glRotatef(90, 1, 0, 0);
		switch ((int)R) {
		case 2:
			//glRotatef(90, 1, 0, 0);
			break;
		case 3:
			//glRotatef(-90, 1, 0, 0);
			break;
		default:
			break;
		}
		//glRotatef(90, 0, 1, 0);
		glRotatef(90, 1, 0, 0);
	}
	else {
		switch ((int)R) {
		case 2:
			//glRotatef(90, 1, 0, 0);
			break;
		case 3:
			//glRotatef(-90, 1, 0, 0);
			break;
		default:
			break;
		}
	}


	//glBindTexture(GL_TEXTURE_2D, texture);


	/*
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
	*/

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnable(GL_NORMAL_ARRAY);
	//glEnable(GL_COLOR_ARRAY);
	//glEnable(GL_VERTEX_ARRAY);

	/*
	glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), arr + 3);
	glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr + 6);
	glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr);
	//*/

	glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), arr2 + 3);
	glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr2 + 6);
	glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr2);

	glPushMatrix();
	//glTranslatef(-2, -2, 0);                // move to bottom-left

	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, indices);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 27 * VertexCount);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 94608);


	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);


}
void CreateSphere(double R, double H, double K, double Z) {

	int n;

	double a;

	double b;



	n = 0;



	//for (b = 0; b <= 90 - space; b += space) {
	for (b = 0; b <= 360 - space; b += space) {


		for (a = 0; a <= 360 - space; a += space) {

			if (fmod(b, 2 * viewingAngle) < viewingAngle + yp && b <= single) {

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
			else {
				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;



				//VERTEX[n].X = R * sin((a) / 180 * PI) * sin((b) / 180 * PI) - H;
				VERTEX[n].X = 0;


				//VERTEX[n].Y = R * cos((a) / 180 * PI) * sin((b) / 180 * PI) + K;
				VERTEX[n].Y = 0;


				//VERTEX[n].Z = R * cos((b) / 180 * PI) - Z;
				VERTEX[n].Z = 0;


				VERTEX[n].V = 0;

				VERTEX[n].U = 0;

				n++;
			}


		}

	}

	// Array of vertices
	for (int i = 0; i < VertexCount; i++) {
		vertices[3 * i] = (GLfloat)VERTEX[i].X;
		vertices[3 * i + 1] = (GLfloat)VERTEX[i].Y;
		vertices[3 * i + 2] = (GLfloat)VERTEX[i].Z;
		//printf("\n%f,%f,%f", vert[3 * i], vert[3 * i + 1], vert[3 * i + 2]);
		norm[3 * i] = vertices[3 * i] / R;
		norm[3 * i + 1] = vertices[3 * i + 1] / R;
		norm[3 * i + 2] = vertices[3 * i + 2] / R;
		col[3 * i] = 1;
		col[3 * i + 1] = 1;
		col[3 * i + 2] = 1;
		arr[9 * i] = vertices[3 * i];
		arr[9 * i + 1] = vertices[3 * i + 1];
		arr[9 * i + 2] = vertices[3 * i + 2];
		arr[9 * i + 3] = norm[3 * i + 0];
		arr[9 * i + 4] = norm[3 * i + 1];
		arr[9 * i + 5] = norm[3 * i + 2];
		arr[9 * i + 6] = col[3 * i + 0];
		arr[9 * i + 7] = col[3 * i + 1];
		arr[9 * i + 8] = col[3 * i + 2];
		//printf("\n%f,%f,%f,%f,%f,%f,%f,%f,%f", arr[9 * i], arr[9 * i + 1], arr[9 * i + 2], arr[9 * i + 3], arr[9 * i + 4], arr[9 * i + 5], arr[9 * i + 6], arr[9 * i + 7], arr[9 * i + 8]);
	}
}

static void
vert(float theta, float phi, int i)
{
	float r = 0.75f;
	r = R;
	float x, y, z, nx, ny, nz;

	nx = sin(DTOR * theta) * cos(DTOR * phi);
	ny = sin(DTOR * phi);
	nz = cos(DTOR * theta) * cos(DTOR * phi);
	glNormal3f(nx, ny, nz);

	x = r * sin(DTOR * theta) * cos(DTOR * phi);
	y = r * sin(DTOR * phi);
	//z = -ZTRANS + r * cos(DTOR * theta) * cos(DTOR * phi);
	z = r * cos(DTOR * theta) * cos(DTOR * phi);
	glVertex4f(x, y, z, 1.0);

	arr2[9 * i] = x;
	arr2[9 * i + 1] = z;
	arr2[9 * i + 2] = y;
	arr2[9 * i + 3] = nx;
	arr2[9 * i + 4] = nz;
	arr2[9 * i + 5] = ny;
	arr2[9 * i + 6] = 1;
	arr2[9 * i + 7] = 1;
	arr2[9 * i + 8] = 1;
	
}

static void
DrawSphere(float del)
{
	//glLoadIdentity();
	float phi, phi2, theta;
	int n = 0;

	glColor4f(1.0, 1.0, 1.0, 1.0);
	for (phi = -0.0f; phi < 360.0f; phi += del) {
		//glBegin(GL_TRIANGLE_STRIP);

		phi2 = phi + del;

		for (theta = -0.0f; theta <= 360.0f; theta += del) {
			if (fmod(theta, 2 * viewingAngle) <= viewingAngle && theta <= single) {
				vert(theta, phi, n);
				vert(theta, phi2, n + 1);
			}
			else {
				glNormal3f(0, 0, 0);
				glVertex4f(0, 0, 0, 0);
				glNormal3f(0, 0, 0);
				glVertex4f(0, 0, 0, 0);
				arr2[9 * n] = 0;
				arr2[9 * n + 1] = 0;
				arr2[9 * n + 2] = 0;
				arr2[9 * n + 3] = 0;
				arr2[9 * n + 4] = 0;
				arr2[9 * n + 5] = 0;
				arr2[9 * n + 6] = 0;
				arr2[9 * n + 7] = 0;
				arr2[9 * n + 8] = 0;
				arr2[9 * (n + 1)] = 0;
				arr2[9 * (n + 1) + 1] = 0;
				arr2[9 * (n + 1) + 2] = 0;
				arr2[9 * (n + 1) + 3] = 0;
				arr2[9 * (n + 1) + 4] = 0;
				arr2[9 * (n + 1) + 5] = 0;
				arr2[9 * (n + 1) + 6] = 0;
				arr2[9 * (n + 1) + 7] = 0;
				arr2[9 * (n + 1) + 8] = 0;
			}
			n += 2;
		}
		//glEnd();
	}
	printf("\n%i", n);
}

void display(void) {

	glClearDepth(1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3f(255, 255, 255); // color white for our rectangle is (255, 255, 255); color yellow is (255, 255, 0)

	glLoadIdentity();


	if (currai6[queueit - 1] == 0 && !written) {
		writeToFile();
		written = 1;
	}
	if (currai6[queueit - 1] != 0) {
		written = 0;
	}

	//glTranslatef(0, 0, -10);
	//glTranslatef(0, 0, -1000);

	/*
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, -1, -1.0, DAQmx_Val_GroupByChannel, currentData, 1400700, &read, NULL));
	goto Skip;

	Error:
	if (DAQmxFailed(error)) {
	DAQmxGetExtendedErrorInfo(errBuff, 2048);
	printf("\nDAQmx Error: %s\n", errBuff);
	}

	Skip:

	for (int i = 0; i < read; i++) {
	if (queueit >= 200100) {
	queueit = 0;
	}
	//printf("%f\n", currentData[i]);
	float64 * tempData = matrixMult((currentData[i] - bias0), (currentData[i + read] - bias1), (currentData[i + (read * 2)] - bias2), (currentData[i + (read * 3)] - bias3), (currentData[i + (read * 4)] - bias4), (currentData[i + (read * 5)] - bias5));
	currai0[queueit] = tempData[0];
	//printf("%f", tempData[0]);
	currai1[queueit] = tempData[1];
	currai2[queueit] = tempData[2];
	currai3[queueit] = tempData[3];
	currai4[queueit] = tempData[4];
	currai5[queueit] = tempData[5];
	currai6[queueit] = (int64)currentData[i + (read * 6)];
	//currxp[queueit] = tempxp;
	currxp[queueit] = OLangle;
	//currxpcl[queueit] = tempxp - aggrlx;
	currxpcl[queueit] = OLangle + CLangle;
	queueit++;
	}
	//*/
	/*
	if (closedLoop) {
	float T = calcFeedback();
	if (abs(T) < threshold) {
	T = 0;
	}
	float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 120.0) * (1.0 / 120.0);
	CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 120.0) * (read * (1.0 / 10000.0) * 120.0) + angVel * (read * (1.0 / 10000.0) * 120.0)) * (180.0 / PI);
	angVel += (float)angAcc * (read * (1.0 / 10000.0) * 120.0);
	printf("\n%f", T);
	}

	//float OLangle = 0;
	if (drifting) {
	OLangle = driftVel * angle;
	//glRotatef(driftVel * angle, horizontal, vertical, spinning);
	}
	else {
	OLangle = (180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0;
	//glRotatef((180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0, horizontal, vertical, spinning);
	}

	if (closedLoop) {
	glRotatef(OLangle + CLangle, horizontal, vertical, spinning);
	//printf("\n%f", CLangle);
	}
	else {
	//glRotatef(OLangle, horizontal, vertical, spinning);
	glRotatef(OLangle, vertical, horizontal, spinning);
	}

	if (!clear) {
	DisplaySphere(5, texture[0]);
	}
	*/

	fps_frames++;
	int delta_t = glutGet(GLUT_ELAPSED_TIME) - fps_start;
	if (delta_t > 1000) {
		//std::cout << double(delta_t) / double(fps_frames) << std::endl;
		std::cout << double(fps_frames) << std::endl;
		//std::cout << delta_t << std::endl;
		fps_frames = 0;
		fps_start = glutGet(GLUT_ELAPSED_TIME);
	}
	int d_t = glutGet(GLUT_ELAPSED_TIME) - t_0;
	//std::cout << d_t << std::endl;
	t_0 = glutGet(GLUT_ELAPSED_TIME);

	//glutSwapBuffers();

	//angle++; //Not really the angle, more like the time step incrementing thing.
	angle += d_t;
	glutSetWindow(window1);
	glutPostRedisplay();
	glutSetWindow(window2);
	glutPostRedisplay();
	glutSetWindow(window3);
	glutPostRedisplay();
}

void display1(void) {

	glClearDepth(1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();	

	//glTranslatef(0, 0, -10);
	//glTranslatef(0, 0, -1000);

	if (closedLoop) {
		float T = calcFeedback();
		if (abs(T) < threshold) {
			T = 0;
		}
		//float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 120.0) * (1.0 / 120.0);
		float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 1000.0) * (1.0 / 1000.0);
		//CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 120.0) * (read * (1.0 / 10000.0) * 120.0) + angVel * (read * (1.0 / 10000.0) * 120.0)) * (180.0 / PI);
		CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 1000.0) * (read * (1.0 / 10000.0) * 1000.0) + angVel * (read * (1.0 / 10000.0) * 1000.0)) * (180.0 / PI);
		//angVel += (float)angAcc * (read * (1.0 / 10000.0) * 120.0);
		angVel += (float)angAcc * (read * (1.0 / 10000.0) * 1000.0);
		printf("\n%f", T);
	}

	//float OLangle = 0;
	if (drifting) {
		OLangle = driftVel * angle;
		//glRotatef(driftVel * angle, horizontal, vertical, spinning);
	}
	else {
		OLangle = (oscillationAmp) * (-1) * cosf(((float)2 * angle * PI / delay)) + (oscillationAmp);
		//glRotatef((180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0, horizontal, vertical, spinning);
		if ((OLangle < (oscillationAmp) && !freq_measured) || (OLangle >(oscillationAmp) && freq_measured)) {
			float delta = (float)(glutGet(GLUT_ELAPSED_TIME) - freq_start) / 1000.0;
			std::cout << 1.0 / (2 * delta) << std::endl;
			freq_start = glutGet(GLUT_ELAPSED_TIME);
			freq_measured = !freq_measured;
		}
		else {
			//freq_measured = 0;
		}
	}

	if (closedLoop) {
		glRotatef(OLangle + CLangle, horizontal, vertical, spinning);
		//printf("\n%f", CLangle);
	}
	else {
		//gluLookAt(0, 0, 0, 0, 0, 1, 1, 0, 0);
		gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
		glRotatef(OLangle, horizontal, vertical, spinning);
		//glRotatef(OLangle, vertical, horizontal, spinning);
	}

	if (!clear) {
		DisplaySphere(1, texture[0]);
		//DrawSphere(5.0);
	}


	glutSwapBuffers();

	angle; //Not really the angle, more like the time step incrementing thing.
}

void display2(void) {

	glClearDepth(1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();



	//glTranslatef(0, 0, -10);
	//glTranslatef(0, 0, -1000);

	if (closedLoop) {
		float T = calcFeedback();
		if (abs(T) < threshold) {
			T = 0;
		}
		//float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 120.0) * (1.0 / 120.0);
		float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 1000.0) * (1.0 / 1000.0);
		//CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 120.0) * (read * (1.0 / 10000.0) * 120.0) + angVel * (read * (1.0 / 10000.0) * 120.0)) * (180.0 / PI);
		CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 1000.0) * (read * (1.0 / 10000.0) * 1000.0) + angVel * (read * (1.0 / 10000.0) * 1000.0)) * (180.0 / PI);
		//angVel += (float)angAcc * (read * (1.0 / 10000.0) * 120.0);
		angVel += (float)angAcc * (read * (1.0 / 10000.0) * 1000.0);
		printf("\n%f", T);
	}

	//float OLangle = 0;
	if (drifting) {
		OLangle = driftVel * angle;
		//glRotatef(driftVel * angle, horizontal, vertical, spinning);
	}
	else {
		OLangle = (oscillationAmp) * (-1) * cosf(((float)2 * angle * PI / delay)) + (oscillationAmp);
		//glRotatef((180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0, horizontal, vertical, spinning);
	}

	if (closedLoop) {
		glRotatef(OLangle + CLangle, horizontal, vertical, spinning);
		//printf("\n%f", CLangle);
	}
	else {
		//gluLookAt(0, 0, 0, 1, 0, 0, 0, 0, -1);
		gluLookAt(0, 0, 0, 1, 0, 0, 0, 1, 0);
		glRotatef(OLangle, horizontal, vertical, spinning);
		//glRotatef(OLangle, vertical, spinning, -horizontal);
	}

	if (!clear) {
		//glRotatef(90, 1, 0, 0);
		DisplaySphere(2, texture[0]);
	}



	glutSwapBuffers();
	wglSwapIntervalEXT(-1);

	//angle++; //Not really the angle, more like the time step incrementing thing.
}

void display3(void) {

	glClearDepth(1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();



	//glTranslatef(0, 0, -10);
	//glTranslatef(0, 0, -1000);

	if (closedLoop) {
		float T = calcFeedback();
		if (abs(T) < threshold) {
			T = 0;
		}
		//float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 120.0) * (1.0 / 120.0);
		float angAcc = (float)(T / (2.43 / 10000000.0)) * (1.0 / 1000.0) * (1.0 / 1000.0);
		//CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 120.0) * (read * (1.0 / 10000.0) * 120.0) + angVel * (read * (1.0 / 10000.0) * 120.0)) * (180.0 / PI);
		CLangle += (float)((angAcc / 2) * (read * (1.0 / 10000.0) * 1000.0) * (read * (1.0 / 10000.0) * 1000.0) + angVel * (read * (1.0 / 10000.0) * 1000.0)) * (180.0 / PI);
		//angVel += (float)angAcc * (read * (1.0 / 10000.0) * 120.0);
		angVel += (float)angAcc * (read * (1.0 / 10000.0) * 1000.0);
		printf("\n%f", T);
	}

	//float OLangle = 0;
	if (drifting) {
		OLangle = driftVel * angle;
		//glRotatef(driftVel * angle, horizontal, vertical, spinning);
	}
	else {
		OLangle = (oscillationAmp) * (-1) * cosf(((float)2 * angle * PI / delay)) + (oscillationAmp);
		//glRotatef((180) * (-1) * cosf(((float)2 * angle * PI / delay)) + 180.0, horizontal, vertical, spinning);
	}

	if (closedLoop) {
		glRotatef(OLangle + CLangle, horizontal, vertical, spinning);
		//printf("\n%f", CLangle);
	}
	else {
		//gluLookAt(0, 0, 0, -1, 0, 0, 0, 0, 1);
		gluLookAt(0, 0, 0, -1, 0, 0, 0, 1, 0);
		glRotatef(OLangle, horizontal, vertical, spinning);
		//glRotatef(OLangle, vertical, spinning, horizontal);
	}

	if (!clear) {
		//glRotatef(-90, 1, 1, 1);
		DisplaySphere(3, texture[0]);
	}



	glutSwapBuffers();
	wglSwapIntervalEXT(-1);

	//angle++; //Not really the angle, more like the time step incrementing thing.
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
	float oscAmp; //oscillation amplitude in degrees
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
		*/
	case 49: //1 will make stimulus single bar
		single = viewingAngle;
		//CreateSphere(R, 0, 0, 0);
		DrawSphere(5.0);
		glutPostRedisplay();
		break;
	case 50: //2 will make stimulus 5-bar grate
		single = 360;
		//CreateSphere(R, 0, 0, 0);
		DrawSphere(5.0);
		glutPostRedisplay();
		break;
	case 86: //V will request viewing angle
		printf("\nInput viewing angle in degrees: ");
		scanf("%f", &degree);
		viewingAngle = degree;
		//barwidth = (float) 0.307975 * tanf(degree * PI / (2.0 * 180.0)) * 1342.281879;
		printf("%f", viewingAngle);
		//CreateSphere(R, 0, 0, 0);
		DrawSphere(5.0);
		glutPostRedisplay();
		break;
	case 118: //v will request viewing angle
		printf("\nInput viewing angle in degrees: ");
		scanf("%f", &degree);
		viewingAngle = degree;
		//barwidth = (float) 0.307975 * tanf(degree * PI / (2.0 * 180.0)) * 1342.281879;
		printf("%f", viewingAngle);
		//CreateSphere(R, 0, 0, 0);
		DrawSphere(5.0);
		glutPostRedisplay();
		break;
	case 70: //F will request frequency
		printf("\nInput oscillation frequency in Hz: ");
		scanf("%f", &frequency);
		//delay = 120.0 / frequency;
		delay = 1000.0 / frequency;
		drifting = 0;
		glutPostRedisplay();
		break;
	case 102: //f will request frequency
		printf("\nInput oscillation frequency in Hz: ");
		scanf("%f", &frequency);
		//delay = 120.0 / frequency;
		delay = 1000.0 / frequency;
		drifting = 0;
		glutPostRedisplay();
		break;
	case 68: //D will request driftDeg
		printf("\nInput drift velocity in degrees/sec: ");
		scanf("%f", &driftDeg);
		//driftVel = 2.0*PI*0.307975*(driftDeg / 360.0)*1342.281879*(1.0 / 120.0);
		//driftVel = driftDeg / 120.0;
		driftVel = driftDeg / 1000.0;
		drifting = 1;
		glutPostRedisplay();
		break;
	case 100: //d will request driftDeg
		printf("\nInput drift velocity in degrees/sec: ");
		scanf("%f", &driftDeg);
		//driftVel = 2.0*PI*0.307975*(driftDeg / 360.0)*1342.281879*(1.0 / 120.0);
		//driftVel = driftDeg / 120.0;
		driftVel = driftDeg / 1000.0;
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
	case 65: //A will request oscillation amplitude in degrees
		printf("\nInput oscillation amplitude in degrees: ");
		scanf("%f", &oscAmp);
		oscillationAmp = oscAmp;
		glutPostRedisplay();
		break;
	case 97: //a will request oscillation amplitude in degrees
		printf("\nInput oscillation amplitude in degrees: ");
		scanf("%f", &oscAmp);
		oscillationAmp = oscAmp;
		glutPostRedisplay();
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

	//texture[0] = LoadTextureRAW(?earth.raw?);

	//CreateSphere(70, 0, 0, 0);
	//CreateSphere(R, 0, 0, 0);
	DrawSphere(5.0);

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnable(GL_NORMAL_ARRAY);
	//glEnable(GL_COLOR_ARRAY);
	//glEnable(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 9 * sizeof(GLfloat), arr + 3);
	glColorPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr + 6);
	glVertexPointer(3, GL_FLOAT, 9 * sizeof(GLfloat), arr);

	glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

}
void reshape(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
	gluPerspective(120, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);


	glMatrixMode(GL_MODELVIEW);
}

void reshape2(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
	gluPerspective(120, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);


	glMatrixMode(GL_MODELVIEW);
}

void reshape3(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	//gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
	gluPerspective(120, (GLfloat)w / (GLfloat)h, 0.1, 1000.0);


	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv) {

	//PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = NULL;
	//PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = NULL;
	float tempWeight;
	//printf("Press left or right arrows to move our rectangle\n");
	/*
	printf("Enter the weight of the moth in grams: ");
	scanf("%f", &tempWeight);
	weight = tempWeight / 1000;
	printf("Please wait about 20 seconds\n");
	//*/
	printf("Please wait about 20 seconds\n");
	/*
	** Add the closed-loop stuff here.
	*/

	// DAQmx analog voltage channel and timing parameters

	/*
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));

	// IMPORTANT
	//changed Dev1 to Dev5 as the connection established. So verify what Dev is being used to update this code. DEV5 is our force torque.
	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0:6", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_ContSamps, 200100));
	// DAQmx Start Code
	DAQmxErrChk(DAQmxStartTask(taskHandle));
	//printf("\ngot here");
	// DAQmx Read Code
	//printf("%f\n", data[0]);
	DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 200100, -1.0, DAQmx_Val_GroupByChannel, data, 1400700, &read, NULL));
	//printf("%f\n", data[0]);
	//printf("\ngot passed through analog");
	//printf("%f\n", data[199999]);
	//bias = data[0];
	// Stop and clear task
	printf("Acquired %d samples\n", (int)read);
	//printf("%f\n", data[199999]);
	//printf("Acquired %d samples\n", (int)read);
	printf("\ngot passed through analog");
	printf("\n%f\n", data[1199999]);
	for (int i = 0; i < read; i++) {
	if (queueit >= 200100) {
	queueit = 0;
	}
	currai0[queueit] = data[i];
	currai1[queueit] = data[i + read];
	currai2[queueit] = data[i + (read * 2)];
	currai3[queueit] = data[i + (read * 3)];
	currai4[queueit] = data[i + (read * 4)];
	currai5[queueit] = data[i + (read * 5)];
	currai6[queueit] = (int64)data[i + (read * 6)]; //our trigger buttom. Must be floored to 0 or 1.
	queueit++;
	}
	bias0 = biasing(currai0);
	bias1 = biasing(currai1);
	bias2 = biasing(currai2);
	bias3 = biasing(currai3);
	bias4 = biasing(currai4);
	bias5 = biasing(currai5);
	writeToFile();
	//*/

	/*
	** This is where the closed-loop stuff ends
	*/

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//glutInitWindowSize(800, 454);
	//glutInitWindowSize(450, 800);
	glutInitWindowSize(144, 256);

	glutInitWindowPosition(2400, 100);
	glutIdleFunc(display);

	glutCreateWindow("A basic OpenGL Window");
	window1 = glutGetWindow();

	glutInitWindowSize(144, 256);
	glutInitWindowPosition(4000, 100);
	glutCreateWindow("Second basic OpenGL Window");
	window2 = glutGetWindow();

	glutInitWindowPosition(5600, 100);
	glutCreateWindow("Don't put me in a box");
	window3 = glutGetWindow();

	glutSetWindow(window1);

	init();



	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		// this is another function from WGL_EXT_swap_control extension
		wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		wglSwapIntervalEXT(-1);
	}

	glutFullScreen(); //This makes shit fullscreen

	glutDisplayFunc(display1);

	glutReshapeFunc(reshape);

	glutKeyboardFunc(letter_pressed);


	// Second window
	glutSetWindow(window2);
	init();


	/*
	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
	// Extension is supported, init pointers.
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	// this is another function from WGL_EXT_swap_control extension
	wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	wglSwapIntervalEXT(-1);
	}
	*/

	glutFullScreen(); //This makes shit fullscreen

	glutDisplayFunc(display2);

	//glutIdleFunc(display2);

	glutReshapeFunc(reshape2);

	//glutKeyboardFunc(letter_pressed);


	// Third window
	glutSetWindow(window3);
	init();


	/*
	if (WGLExtensionSupported("WGL_EXT_swap_control"))
	{
	// Extension is supported, init pointers.
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

	// this is another function from WGL_EXT_swap_control extension
	wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	wglSwapIntervalEXT(-1);
	}
	*/

	glutFullScreen(); //This makes shit fullscreen

	glutDisplayFunc(display3);

	//glutIdleFunc(display2);

	glutReshapeFunc(reshape3);

	//glutKeyboardFunc(letter_pressed);

	goto Skip;
Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0) {
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		printf("\nDAQmx Error: %s\n", errBuff);
	getchar();
Skip:
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