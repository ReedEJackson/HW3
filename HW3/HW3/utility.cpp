#include <string.h>
#include <iostream>
#include <fstream>
#include "utility.h"
using namespace std;

// Read a shader file into a text buffer
// Returns pointer to the heap-created buffer
// See Angel 6th ed.
char* readShaderFile(const char* shaderFileName)
{
	ifstream f(shaderFileName);
	int fileSize;

	if (!f) { return NULL; }
	f.seekg(0, f.end);
	fileSize = f.tellg();
	char* buffer = new char[fileSize + 1];

	f.seekg(0, f.beg);
	f.read(buffer, fileSize);
	buffer[fileSize] = '\0';
	f.close();

	return buffer;
}

// Parse argv[] for -v and -f
void getArgs(int argc, char** argv,
	char* vertexShaderName, char* fragmentShaderName)
{
	for (int i = 1; i < argc; i += 2) {
		string str1(argv[i]), str2(argv[i + 1]);
		if (str1 == "-v") strcpy_s(vertexShaderName, 256, str2.c_str());
		else if (str1 == "-f") strcpy_s(fragmentShaderName, 256, str2.c_str());
	}
}

int readVertexColor(char* dataFileName, VertexColor v[])
{
	ifstream f(dataFileName);
	int i = 0;
	f >> v[i].point.x;
	while (!f.eof()) {
		f >> v[i].point.y >> v[i].point.z;
		f >> v[i].color.r >> v[i].color.g >> v[i].color.b;
		i++;
		f >> v[i].point.x;
	}
	return(i);
}

int readVertexNormalColor(char* dataFileName, VertexNormalColor v[])
{
	ifstream f(dataFileName);
	int i = 0;
	f >> v[i].point.x;
	while (!f.eof()) {
		f >> v[i].point.y >> v[i].point.z;
		f >> v[i].normal.x >> v[i].normal.y >> v[i].normal.z;
		f >> v[i].color.r >> v[i].color.g >> v[i].color.b;
		i++;
		f >> v[i].point.x;
	}
	return(i);
}

int readVertexNormal(char* dataFileName, VertexNormal v[]) {
	ifstream f(dataFileName);
	int i = 0;
	f >> v[i].point.x;
	while (!f.eof()) {
		f >> v[i].point.y >> v[i].point.z;
		f >> v[i].normal.x >> v[i].normal.y >> v[i].normal.z;
		i++;
		f >> v[i].point.x;
	}
	return(i);
}

int readMaterials(char* dataFileName, MaterialStruct v[])
{
	ifstream f(dataFileName);
	int i = 0;
	f >> v[i].ambient.x;
	while (!f.eof()) {
		f >> v[i].ambient.y >> v[i].ambient.z;
		f >> v[i].diffuse.x >> v[i].diffuse.y >> v[i].diffuse.z;
		f >> v[i].specular.x >> v[i].specular.y >> v[i].specular.z;
		f >> v[i].shininess;
		i++;
		f >> v[i].ambient.x;
	}
	return(i);
}