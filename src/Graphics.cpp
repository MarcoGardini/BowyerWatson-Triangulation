#include "Graphics.hpp"

#define TRIANGLES_PER_POINT 3 // should be 2, but keeping it safer
#define VERTICES_PER_TRIANGLE 3
#define COORDS_PER_VERTEX 3 // which are x, y and color of each vertex

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

GraphicDriver::GraphicDriver(int maxPoints)
{
	backgroundColor = glm::vec3(0.f, 0.0f, 0.f);
	mainColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
	calculatedAlpha = false;
	technique = TECHNIQUE::AVERAGE_FROM_CENTER;

	// initialize buffer for vertices
	verticesBuffer = new float[maxPoints * TRIANGLES_PER_POINT * VERTICES_PER_TRIANGLE * COORDS_PER_VERTEX];

	// INIT OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 16);

	// Create Window
	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);

	// disable vSync
	glfwSwapInterval(0);

	// link functions to glfw callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// INIT GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// tell OPENGL how to draw
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_FILL, GL_LINE, GL_POINT

	// enables and set blend func
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

	// set Viewport dimension for screen transform
	glViewport(0, 0, WIDTH, HEIGHT);

	// shader and glObjects generation
	shader = new Shader("assets\\shader.vert", "assets\\shader.frag");	
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

}

void GraphicDriver::Render()
{
	// stage reset
	glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO);
	shader->use();
	shader->setVec4("triangColor", mainColor);
	shader->setBool("calcAlpha", calculatedAlpha);
	glDrawArrays(GL_TRIANGLES, 0, howManyCoordinates);
	// swap buffers and poll events
	glfwSwapBuffers(window);
}

// on resize, set viewport to appropriate width and height
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// keep  original aspect ratio
	height = HEIGHT * width / WIDTH;
	glViewport(0, 0, width, height);
	glfwSetWindowSize(window, width, height);
}

float findVerticalBalance(Triangle* triangle);

void GraphicDriver::PopulateBuffer(std::unordered_set<Triangle*> triangulation)
{
	int i = 0;
	float thisColor;
	srand(1000);
	// fill vertices array with triangles' vertices and colors
	for (Triangle* triangle : triangulation)
	{
		switch (technique)
		{
		case TECHNIQUE::AVERAGE_FROM_CENTER:
			thisColor = abs(glm::length(triangle->pointA) + glm::length(triangle->pointB) + glm::length(triangle->pointC)) / 3.f;
			break;
		case TECHNIQUE::AVERAGE_Y:
			thisColor = (triangle->pointA.y + triangle->pointB.y + triangle->pointC.y) / 3.f / 2.f + .5f;
			break;
		case TECHNIQUE::AVERAGE_Y_BALANCE:
			thisColor = findVerticalBalance(triangle);
			break;
		case TECHNIQUE::AREA:
			thisColor = abs(triangle->pointA.x * (triangle->pointB.y - triangle->pointC.y) +
				triangle->pointB.x * (triangle->pointC.y - triangle->pointA.y) +
				triangle->pointC.x * (triangle->pointA.y - triangle->pointB.y)) / 2.f;
			thisColor *= 100;
			break;
		default:
			thisColor = rand() / (float)(RAND_MAX);
			break;
		}
		verticesBuffer[i++] = triangle->pointA.x;
		verticesBuffer[i++] = triangle->pointA.y;
		verticesBuffer[i++] = invertColor ? 1 - thisColor : thisColor;
		verticesBuffer[i++] = triangle->pointB.x;
		verticesBuffer[i++] = triangle->pointB.y;
		verticesBuffer[i++] = invertColor ? 1 - thisColor : thisColor;
		verticesBuffer[i++] = triangle->pointC.x;
		verticesBuffer[i++] = triangle->pointC.y;
		verticesBuffer[i++] = invertColor ? 1 - thisColor : thisColor;
	}

	howManyTriangles = triangulation.size();
	howManyCoordinates = howManyTriangles * VERTICES_PER_TRIANGLE * COORDS_PER_VERTEX;
	
	// fill glBuffer
	glBufferData(GL_ARRAY_BUFFER, howManyCoordinates * sizeof(float), verticesBuffer, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
}

float findVerticalBalance(Triangle* triangle)
{
	float highest = triangle->pointA.y, lowest = triangle->pointB.y, balance = triangle->pointC.y;
	// find max
	if (lowest > highest) std::swap(lowest, highest);
	if (balance > highest) std::swap(balance, highest);
	// find min
	if (highest < lowest) std::swap(lowest, highest);
	if (balance < lowest) std::swap(balance, lowest);
	// highest = 1, lowest = 0, balance is the resulting weight
	balance = (highest - balance) / (highest - lowest);

	return balance;
}