#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32.lib")

#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
// For file-reading functions
#include "utility.h"
using namespace std;

// Global variables
// Window size
int windowWidth = 800,
windowHeight = 600,
windowID = 0;

// Clear color
struct Color {
	float red = 0.0,
		green = 0.0,
		blue = 0.0;
};

Color clearColor;

GLuint vao, vbo, program;  // Global names
						   //Shader names
GLuint shaderProgram, vertexShader, fragmentShader, pointCount;

// Eye, lookat, and up defaults
glm::vec3 eye(8.0f, 6.0f, 6.0f),
lookat(2.0f, 0.0f, 2.0f),
up(0.0f, 1.0f, 0.0f);
// Perspective defaults
float viewAngle(glm::radians(45.0f)),
aspect(1.0*windowWidth / windowHeight),
nearPlane(0.1f),
farPlane(100.0f);
// Transformation matrices
glm::mat4 viewMatrix(1.0);
glm::mat4 modelMatrix(1.0);
glm::mat4 projectionMatrix(1.0);
glm::mat4 normalMatrix(1.0);
glm::mat4 ttm(1.0);
glm::vec3 trans(0.1f, 0.1f, 0.0f);
GLfloat angle = glm::radians(10.0f);

const int MAXGRIDVAL = 100;
const int MAXPOINTS = 10000;
VertexNormal *myarray = new VertexNormal[MAXPOINTS];
int myarrayCounter = 0;

float stepSize = 1.0f; //Variable to control the grip step size

MaterialStruct material1 = {
	glm::vec3(0.3f, 0.28f, 0.2f),
	glm::vec3(1.0f, 0.922f, 0.804f),
	glm::vec3(1.0f, 0.99f, 0.9f),
	7
};

SpotlightStruct light1 = {
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.416f, 0.055f, 0.710f),
    glm::normalize(glm::vec3(0.0f, 0.2f, 1.0f)),
    100.0f,
    0.99f
};

GLuint location[14];
// Window title string
const char* WINDOW_TITLE = "Homework 3";

#pragma region Function prototypes

//Basic setup
void Initialize(int, char*[]);
void setupShaders(int, char**);
void setupData(void);
void ResizeCallback(int, int);
void DisplayCallback(void);
void setupUniforms(void);

//Shaders
GLuint compileShader(GLenum type, const GLchar* source);
GLuint linkShader(GLuint vertexShader, GLuint fragmentShader);

//Controls
void KeyboardCallback(unsigned char key, int x, int y);
void SpecialKeyCallback(int key, int x, int y);

//Creating and drawing
void setupEnable();
void drawSurface();

int makeSurface(VertexNormal* surfaceArray, int arrayCounter);
glm::vec3 getSurfacePoint(float x, float z);
glm::vec3 getSurfaceNormal(float x, float z);

#pragma endregion


int main(int argc, char* argv[])
{
	glewExperimental = GL_TRUE;

	// Set up the
	Initialize(argc, argv);
	setupShaders(argc, argv);
	setupData();
	glutMainLoop();
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &vbo);
	exit(EXIT_SUCCESS);

} // main( )

void Initialize(int argc, char* argv[])
{
	// Initialize OpenGL
	glutInit(&argc, argv);

	// Set some context
	glutInitContextVersion(4, 4);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// Set close option
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// Set the display mode
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);

	// Set the initial clear color
	glClearColor(clearColor.red, clearColor.green, clearColor.blue, 0.0f);

	// Create a window
	glutInitWindowSize(windowWidth, windowHeight);
	windowID = glutCreateWindow(WINDOW_TITLE);

	if (windowID < 1) {
		cout << "ERROR: Could not create a new rendering window." << endl;
		exit(EXIT_FAILURE);
	}
	glewExperimental = TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Could not initialize glew!" << endl;
		exit(EXIT_FAILURE);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LESS);

	// Set the callback functions
	glutReshapeFunc(ResizeCallback);
	glutDisplayFunc(DisplayCallback);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(SpecialKeyCallback);

	// Just to see it
	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

} // InitializeWindow( )

void setupShaders(int argc, char** argv)
{

	char* vertexFileName = new char[256];
	char* fragmentFileName = new char[256];
	getArgs(argc, argv, vertexFileName, fragmentFileName);
	GLchar* vertexSource = (GLchar*)readShaderFile(vertexFileName);
	if (vertexSource == NULL) {
		cout << "read error" << endl;
	}
	else {
		//cout << vertexSource << endl;
	}

	vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);

	GLchar* fragmentSource = (GLchar*)readShaderFile(fragmentFileName);

	fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

	//START DEBUG
	GLint b;
	GLint length = 200;
	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
	char* log = (char*)malloc(length);
	glGetShaderInfoLog(fragmentShader, 200, &length, log);
	printf("Log file: ");
	if (length>1)
		printf("%s\n", log);
	//END DEBUG

	shaderProgram = linkShader(vertexShader, fragmentShader);
	glUseProgram(shaderProgram);

} // setupShaders( )

void setupData(void)
{
	// Set up surface data
	pointCount = makeSurface(myarray, myarrayCounter);

	viewMatrix = glm::lookAt(eye, lookat, up);
	projectionMatrix = glm::perspective(viewAngle, aspect, nearPlane, farPlane);

	glGenVertexArrays(1, &vao); // Generate name
	glBindVertexArray(vao); // Bind name: what follows belongs to *this* VAO
	glGenBuffers(1, &vbo); // Generate name
	glBindBuffer(GL_ARRAY_BUFFER, vbo); // Bind name
										// Now transfer the actual data
	glBufferData(GL_ARRAY_BUFFER, MAXPOINTS * sizeof(VertexNormal), myarray, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

} // setupData()

void setupUniforms()
{
	location[1] = glGetUniformLocation(program, "modelMatrix");
	if (location[1] >= 0) {
		glUniformMatrix4fv(location[1], 1, GL_FALSE, &modelMatrix[0][0]);
	}
	location[2] = glGetUniformLocation(program, "viewMatrix");
	if (location[2] >= 0) {
		glUniformMatrix4fv(location[2], 1, GL_FALSE, &viewMatrix[0][0]);
	}
	location[3] = glGetUniformLocation(program, "projectionMatrix");
	if (location[3] >= 0) {
		glUniformMatrix4fv(location[3], 1, GL_FALSE, &projectionMatrix[0][0]);
	}
	location[4] = glGetUniformLocation(program, "normalMatrix");
	if (location[4] >= 0) {
		glUniformMatrix4fv(location[4], 1, GL_FALSE, &normalMatrix[0][0]);
	}
	location[5] = glGetUniformLocation(program, "light1.position");
	if (location[5] >= 0) {
		glUniform3fv(location[5], 1, &light1.position[0]);
	}
	location[6] = glGetUniformLocation(program, "light1.intensity");
	if (location[6] >= 0) {
		glUniform3fv(location[6], 1, &light1.intensity[0]);
	}
    location[7] = glGetUniformLocation(program, "light1.direction");
    if (location[7] >= 0) {
        glUniform3fv(location[7], 1, &light1.direction[0]);
    }
    location[8] = glGetUniformLocation(program, "light1.exponent");
    if (location[8] >= 0) {
        glUniform1f(location[8], light1.exponent);
    }
    location[9] = glGetUniformLocation(program, "light1.cutoffCosine");
    if (location[9] >= 0) {
        glUniform1f(location[9], light1.cutoffCosine);
    }
	location[10] = glGetUniformLocation(program, "material1.ambient");
	if (location[10] >= 0) {
		glUniform3fv(location[10], 1, &material1.ambient[0]);
	}
	location[11] = glGetUniformLocation(program, "material1.diffuse");
	if (location[11] >= 0) {
		glUniform3fv(location[11], 1, &material1.diffuse[0]);
	}
	location[12] = glGetUniformLocation(program, "material1.specular");
	if (location[12] >= 0) {
		glUniform3fv(location[12], 1, &material1.specular[0]);
	}
	location[13] = glGetUniformLocation(program, "material1.shininess");
	if (location[13] >= 0) {
		glUniform1f(location[13], material1.shininess);
	}
}

void ResizeCallback(int width, int height)
{

	// width and height are set by the window manager
	// Store new values in the globals
	windowWidth = width;
	windowHeight = height;
	aspect = 1.0*windowWidth / windowHeight;
	glViewport(0, 0, windowWidth, windowHeight);

} // ResizeCallback( )

void DisplayCallback(void)
{

	// Clear the display, swap the buffers, redraw
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewMatrix = glm::lookAt(eye, lookat, up);
	projectionMatrix = glm::perspective(viewAngle, aspect, nearPlane, farPlane);
	normalMatrix = glm::transpose(glm::inverse(modelMatrix));

	setupUniforms();
	glBindVertexArray(vao);

	drawSurface();

	glBindVertexArray(0);
	glutSwapBuffers();
	glutPostRedisplay();

} // DisplayCallback( )

GLuint compileShader(GLenum type, const GLchar* source)
{
	GLint length = sizeof(GLchar) * strlen(source);
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, &length);
	glCompileShader(shader);
	GLint shaderOk = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderOk);
	if (!shaderOk) {
		cout << "makeShader: Failed to compile the shader!" << endl;
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

GLuint linkShader(GLuint vertexShader, GLuint fragmentShader)
{
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint programOk = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &programOk);
	if (!programOk) {
		cout << "linkShader: Failed to link the shader!" << endl;;
		glDeleteShader(program);
		program = 0;
	}
	return program;
}

void KeyboardCallback(unsigned char key, int x, int y)
{
	glm::mat4 rot(1.0), temp(1.0);
	glm::vec4 a;
	switch (toupper(key)) {
	case 'Q': exit(EXIT_SUCCESS);
		break;
	case 'W':
		eye = eye + 0.1f*(lookat - eye);
		break;
	case 'S':
		eye = eye - 0.1f*(lookat - eye);
		break;
	case 'R':
		rot = glm::rotate(rot, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'F':
		rot = glm::rotate(rot, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'T':
		rot = glm::rotate(rot, glm::radians(10.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'G':
		rot = glm::rotate(rot, glm::radians(-10.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'Y':
		rot = glm::rotate(rot, glm::radians(10.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'H':
		rot = glm::rotate(rot, glm::radians(-10.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		a = rot*glm::vec4(eye, 1.0f);
		eye.x = a.x; eye.y = a.y; eye.z = a.z;
		break;
	case 'U':
		viewAngle += glm::radians(1.0f);
		break;
	case 'J':
		viewAngle -= glm::radians(1.0f);
		break;
	case 'I':
		nearPlane += 0.5f;
		break;
	case 'K':
		nearPlane -= 0.5f;
		break;
	case 'Z':
		temp = glm::rotate(temp, glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = temp*modelMatrix;
		break;
	case 'X':
		temp = glm::rotate(temp, glm::radians(-10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		modelMatrix = temp*modelMatrix;
		break;
	case 'C':
		temp = glm::rotate(temp, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = temp*modelMatrix;
		break;
	case 'V':
		temp = glm::rotate(temp, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = temp*modelMatrix;
		break;
	case 'B':
		temp = glm::rotate(temp, glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = temp*modelMatrix;
		break;
	case 'N':
		temp = glm::rotate(temp, glm::radians(-10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = temp*modelMatrix;
		break;
    case 'O':
        // use larger grid
        stepSize = stepSize < 2.0f ? stepSize * 2.0f : 2.0f;
        pointCount = makeSurface(myarray, 0);
        break;
    case 'L':
        // use smaller grid
        stepSize = stepSize > 0.25f ? stepSize * 0.5f : 0.25f;
        pointCount = makeSurface(myarray, 0);
        break;
	}

	glutPostRedisplay();
} // KeyboardCallback( )

void SpecialKeyCallback(int key, int x, int y)
{
	glm::vec3 vec;
	switch (key) {
	case GLUT_KEY_UP:
		lookat = lookat + 0.1f*up;
		break;
	case GLUT_KEY_DOWN:
		lookat = lookat - 0.1f*up;
		break;
	case GLUT_KEY_LEFT:
		vec = lookat - eye;
		lookat = lookat - 0.1f*glm::cross(up, vec);
		break;
	case GLUT_KEY_RIGHT:
		vec = lookat - eye;
		lookat = lookat + 0.1f*glm::cross(up, vec);
		break;
	}
	glutPostRedisplay();
}

void setupEnable() {
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,		// Use buffer 0
		3, 		// Using vec3 data
		GL_FLOAT, 	// of floats
		GL_FALSE,	// not normalized
		sizeof(VertexNormal),
		(void*)0        // Start at the beginning
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(VertexNormal),
		(void*)(sizeof(GLfloat) * 3)
	);
}

void drawSurface() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glUniform3fv(location[10], 1, &material1.ambient[0]);
	glUniform3fv(location[11], 1, &material1.diffuse[0]);
	glUniform3fv(location[12], 1, &material1.specular[0]);
	glUniform1f(location[13], material1.shininess);

	glBufferData(GL_ARRAY_BUFFER, MAXPOINTS * sizeof(VertexNormal),
		myarray, GL_STATIC_DRAW);
	setupEnable();
	glDrawArrays(GL_TRIANGLES, 0, pointCount);
}

int makeSurface(VertexNormal* surfaceArray, int arrayCounter)
{
	glm::vec3 stepSurface[MAXGRIDVAL][MAXGRIDVAL];
	glm::vec3 stepNormals[MAXGRIDVAL][MAXGRIDVAL];

	//Populate array with vertices and normals
	int x = 0;
	for (int row = 0.0f; row <= 4.0f / stepSize; row++)
	{
		x = row * stepSize;
		for (int col = 0.0f; col <= 4.0f / stepSize; col++)
		{
			stepSurface[row][col] = getSurfacePoint(x, col * stepSize);
			stepNormals[row][col] = getSurfaceNormal(x, col * stepSize);
		}
	}
	
	//Generate triangles
	glm::vec3 triangle1;
	glm::vec3 triangle2;
	for (int row = 0.0f; row < 4.0f / stepSize; row++)
	{
		for (int col = 0.0f; col < 4.0f / stepSize; col++)
		{
            #pragma region Add Points

			surfaceArray[arrayCounter].point = stepSurface[row][col];
			surfaceArray[arrayCounter++].normal = stepNormals[row][col];

			surfaceArray[arrayCounter].point = stepSurface[row + 1][col + 1];
			surfaceArray[arrayCounter++].normal = stepNormals[row + 1][col + 1];

			surfaceArray[arrayCounter].point = stepSurface[row + 1][col];
			surfaceArray[arrayCounter++].normal = stepNormals[row + 1][col];

			surfaceArray[arrayCounter].point = stepSurface[row][col];
			surfaceArray[arrayCounter++].normal = stepNormals[row][col];

			surfaceArray[arrayCounter].point = stepSurface[row][col + 1];
			surfaceArray[arrayCounter++].normal = stepNormals[row][col + 1];

			surfaceArray[arrayCounter].point = stepSurface[row + 1][col + 1];
			surfaceArray[arrayCounter++].normal = stepNormals[row + 1][col + 1];

            #pragma endregion
		}
	}

	return arrayCounter; //Return number of vertices
}

glm::vec3 getSurfacePoint(float x, float z)
{
	float y = x * (z + 1.0f) * (x*x - (z + 1.0f) * (z + 1.0f)) / 50.0f; //equation given in assignment
	return glm::vec3(x, y, z);
}

glm::vec3 getSurfaceNormal(float x, float z)
{
	float partialWrtX, partialWrtZ;
	glm::vec3 normal;

	partialWrtX = (z + 1.0f) * (3.0f * x*x - (z + 1.0f) * (z + 1.0f)) / 50.0f;
	partialWrtZ = x * (x*x - 3.0f * (z + 1.0f) * (z + 1.0f)) / 50.0f;

	normal = glm::vec3(-partialWrtX, 1, -partialWrtZ);
	return glm::normalize(normal); //ensure normal is unitized
}
