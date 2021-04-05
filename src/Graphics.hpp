#pragma once

#include <iostream>
#include <unordered_set>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Triangulation.hpp"

extern const int MAX_SIZE;

// window size
const uint32_t WIDTH{ 1920 }, HEIGHT{ 1080 };

class GraphicDriver
{
private: 
	uint32_t VAO, VBO;
	Shader* shader;

public:
	enum class TECHNIQUE {
		AVERAGE_FROM_CENTER,
		AVERAGE_Y,
		AVERAGE_Y_BALANCE,
		AREA,
		RANDOM,
		MAX_TECHNIQUE,
	};

	float* verticesBuffer;
	int howManyCoordinates, howManyTriangles;
	glm::vec3 backgroundColor;
	glm::vec4 mainColor;
	bool calculatedAlpha;
	bool invertColor;
	TECHNIQUE technique;
	GLFWwindow* window;

	// constructor
	GraphicDriver(int howMany);

	// converts triangles into vertices and colors for rendering + send to GPU
	void PopulateBuffer(std::unordered_set<Triangle*> triangulation);

	// draw and swap, after setting the shader
	void Render();
};