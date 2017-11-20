#pragma once
#include <glm\glm.hpp>
char* readShaderFile(const char* shaderFileName);
void getArgs(int argc, char** argv,
	char* vertexShaderName, char* fragmentShaderName);

struct VertexColor {
	glm::vec3 point;
	glm::vec3 color;
};

struct VertexNormalColor {
	glm::vec3 point;
	glm::vec3 normal;
	glm::vec3 color;
};

struct VertexNormal {
	glm::vec3 point;
	glm::vec3 normal;
};

struct LightStruct {
	glm::vec3 position;
	glm::vec3 intensity;
};

struct SpotlightStruct {
    glm::vec3 position;
    glm::vec3 intensity;
    glm::vec3 direction;
    float exponent;
    float cutoffCosine;
};

struct MaterialStruct {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;
};

int readVertexColor(char* dataFileName, VertexColor v[]);
int readVertexNormalColor(char* dataFileName, VertexNormalColor v[]);
int readVertexNormal(char* dataFileName, VertexNormal v[]);
int readMaterials(char* dataFileName, MaterialStruct v[]);
