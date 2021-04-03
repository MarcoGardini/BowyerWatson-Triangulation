#include "TriangulationManager.hpp"

#include <chrono>

TriangulationManager::TriangulationManager()
{
	pointSpeed = 0.02f;
	triangulation = new Triangulation(pointSpeed, MAX_TRIANGLES);
	graphicDriver = new GraphicDriver(TRIANGULATION_MAX_POINTS);
}

void TriangulationManager::Run()
{
	// frameTime handling
	bool firstLoop = true;
	auto frameTime = std::chrono::high_resolution_clock::now();
	
	// repeated keystrokes handling
	auto keyPressTime = std::chrono::high_resolution_clock::now();
	bool freeToPressKey = true;

	// MAIN LOOP
	while (!glfwWindowShouldClose(graphicDriver->window))
	{
		// avoid quick repeated keystrokes by timing out the input handling function
		if (freeToPressKey && ProcessInput())
		{
			freeToPressKey = false;
			keyPressTime = std::chrono::high_resolution_clock::now();
		}
		
		// update points and create the triangulation
		triangulation->Update(deltaTime);

		// send triangulation to graphics and render
		graphicDriver->PopulateBuffer(triangulation->triangulation);		
		graphicDriver->Render();

		// event polling
		glfwPollEvents();

		// time management 
		if (firstLoop)
		{
			deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - frameTime).count() / 1000000000.f;
			frameTime = std::chrono::high_resolution_clock::now();
			firstLoop = false;
		}
		else
		{
			deltaTime += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - frameTime).count() / 1000000000.f;
			deltaTime /= 2;
			frameTime = std::chrono::high_resolution_clock::now();
		}

		// release input handling from timeout
		if (!freeToPressKey && std::chrono::duration_cast<std::chrono::milliseconds>
			(std::chrono::high_resolution_clock::now() - keyPressTime).count() > MS_TO_REPEAT_KEY_PRESS)
		{
			freeToPressKey = true;
			keyPressTime = std::chrono::high_resolution_clock::now();
		}
	}
}

bool TriangulationManager::ProcessInput()
{	
	// self explanatory input handling, see Controls.txt
	// each input cause an action and logs on the console
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(graphicDriver->window, true);
		std::cout << "EXITING..." << std::endl;
		return true;
	}
	// ALGORITHM
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		triangulation->algorithm = Triangulation::ALGORITHM::BowyerWatson;
		std::cout << "ALGORITHM SET -> BOWYER-WATSON" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_F2) == GLFW_PRESS)
	{
		triangulation->algorithm = Triangulation::ALGORITHM::ContiguousBowyerWatson;
		std::cout << "ALGORITHM SET -> BOWYER-WATSON WITH CONTIGUOUS TRIANGLES CHECK" << std::endl;
		return true;
	}
	// POINTS
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		triangulation->GeneratePoints(pointSpeed);
		std::cout << "GENERATED NEW POINTS" << std::endl;		
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_1) == GLFW_PRESS)
	{
		if (++triangulation->howManyPoints > TRIANGULATION_MAX_POINTS) triangulation->howManyPoints = TRIANGULATION_MAX_POINTS;
		std::cout << "ADDING 1 POINT, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (--triangulation->howManyPoints < TRIANGULATION_MIN_POINTS) triangulation->howManyPoints = TRIANGULATION_MIN_POINTS;
		std::cout << "REMOVING 1 POINT, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_2) == GLFW_PRESS)
	{
		triangulation->howManyPoints += 10;
		if (triangulation->howManyPoints > TRIANGULATION_MAX_POINTS) triangulation->howManyPoints = TRIANGULATION_MAX_POINTS;
		std::cout << "ADDING 10 POINTS, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_W) == GLFW_PRESS)
	{
		triangulation->howManyPoints -= 10;
		if (triangulation->howManyPoints < TRIANGULATION_MIN_POINTS) triangulation->howManyPoints = TRIANGULATION_MIN_POINTS;
		std::cout << "REMOVING 10 POINTS, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_3) == GLFW_PRESS)
	{
		triangulation->howManyPoints += 100;
		if (triangulation->howManyPoints > TRIANGULATION_MAX_POINTS)
			triangulation->howManyPoints = TRIANGULATION_MAX_POINTS;
		std::cout << "ADDING 100 POINTS, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_E) == GLFW_PRESS)
	{
		triangulation->howManyPoints -= 100;
		if (triangulation->howManyPoints < TRIANGULATION_MIN_POINTS) triangulation->howManyPoints = TRIANGULATION_MIN_POINTS;
		std::cout << "REMOVING 100 POINTS, CURRENT POINTS: " << triangulation->howManyPoints << std::endl;
		triangulation->GeneratePoints(pointSpeed);
		return true;
	}
	// COLORS
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.r += 0.1f;
		else
			graphicDriver->backgroundColor.r += 0.01f;
		if (graphicDriver->backgroundColor.r > 1.0f) graphicDriver->backgroundColor.r = 1.0f;
		std::cout << "BACKGROUND COLOR, RED CHANNEL: " << graphicDriver->backgroundColor.r << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.r -= 0.1f;
		else
			graphicDriver->backgroundColor.r -= 0.01f;
		if (graphicDriver->backgroundColor.r < 0.0f) graphicDriver->backgroundColor.r = 0.0f;
		std::cout << "BACKGROUND COLOR, RED CHANNEL: " << graphicDriver->backgroundColor.r << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.g += 0.1f;
		else
			graphicDriver->backgroundColor.g += 0.01f;
		if (graphicDriver->backgroundColor.g > 1.0f) graphicDriver->backgroundColor.g = 1.0f;
		std::cout << "BACKGROUND COLOR, GREEN CHANNEL: " << graphicDriver->backgroundColor.g << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_X) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.g -= 0.1f;
		else
			graphicDriver->backgroundColor.g -= 0.01f;
		if (graphicDriver->backgroundColor.g < 0.0f) graphicDriver->backgroundColor.g = 0.0f;
		std::cout << "BACKGROUND COLOR, GREEN CHANNEL: " << graphicDriver->backgroundColor.g << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.b += 0.1f;
		else
			graphicDriver->backgroundColor.b += 0.01f;
		if (graphicDriver->backgroundColor.b > 1.0f) graphicDriver->backgroundColor.b = 1.0f;
		std::cout << "BACKGROUND COLOR, BLUE CHANNEL: " << graphicDriver->backgroundColor.b << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_C) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->backgroundColor.b -= 0.1f;
		else
			graphicDriver->backgroundColor.b -= 0.01f;
		if (graphicDriver->backgroundColor.b < 0.0f) graphicDriver->backgroundColor.b = 0.0f;
		std::cout << "BACKGROUND COLOR, BLUE CHANNEL: " << graphicDriver->backgroundColor.b << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_F) == GLFW_PRESS)
	{                                                                            
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.r += 0.1f;
		else
			graphicDriver->mainColor.r += 0.01f;
		if (graphicDriver->mainColor.r > 1.0f) graphicDriver->mainColor.r = 1.0f;
		std::cout << "MAIN COLOR, RED CHANNEL: " << graphicDriver->mainColor.r << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_V) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.r -= 0.1f;
		else
			graphicDriver->mainColor.r -= 0.01f;
		if (graphicDriver->mainColor.r < 0.0f) graphicDriver->mainColor.r = 0.0f;
		std::cout << "MAIN COLOR, RED CHANNEL: " << graphicDriver->mainColor.r << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_G) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.g += 0.1f;
		else
			graphicDriver->mainColor.g += 0.01f;
		if (graphicDriver->mainColor.g > 1.0f) graphicDriver->mainColor.g = 1.0f;
		std::cout << "MAIN COLOR, GREEN CHANNEL: " << graphicDriver->mainColor.g << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_B) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.g -= 0.1f;
		else
			graphicDriver->mainColor.g -= 0.01f;
		if (graphicDriver->mainColor.g < 0.0f) graphicDriver->mainColor.g = 0.0f;
		std::cout << "MAIN COLOR, GREEN CHANNEL: " << graphicDriver->mainColor.g << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_H) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.b += 0.1f;
		else
			graphicDriver->mainColor.b += 0.01f;
		if (graphicDriver->mainColor.b > 1.0f) graphicDriver->mainColor.b = 1.0f;
		std::cout << "MAIN COLOR, BLUE CHANNEL: " << graphicDriver->mainColor.b << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_N) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.b -= 0.1f;
		else
			graphicDriver->mainColor.b -= 0.01f;
		if (graphicDriver->mainColor.b < 0.0f) graphicDriver->mainColor.b = 0.0f;
		std::cout << "MAIN COLOR, BLUE CHANNEL: " << graphicDriver->mainColor.b << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_J) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.a += 0.1f;
		else
			graphicDriver->mainColor.a += 0.01f;
		if (graphicDriver->mainColor.a > 1.0f) graphicDriver->mainColor.a = 1.0f;
		std::cout << "MAIN COLOR, ALPHA CHANNEL: " << graphicDriver->mainColor.a << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_M) == GLFW_PRESS)
	{
		if (glfwGetKey(graphicDriver->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			graphicDriver->mainColor.a -= 0.1f;
		else
			graphicDriver->mainColor.a -= 0.01f;
		if (graphicDriver->mainColor.a < 0.0f) graphicDriver->mainColor.a = 0.0f;
		std::cout << "MAIN COLOR, ALPHA CHANNEL: " << graphicDriver->mainColor.a << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		graphicDriver->calculatedAlpha = !graphicDriver->calculatedAlpha;
		std::cout << "CALCULATED ALPHA: " << graphicDriver->calculatedAlpha << std::endl;
		return true;
	}
	// COLOR TECHNIQUES
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_1) == GLFW_PRESS)
	{
		graphicDriver->technique = GraphicDriver::TECHNIQUE::AVERAGE_FROM_CENTER;
		std::cout << "COLOR TECHNIQUE SET TO: AVERAGE DISTANCE FROM CENTER" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_2) == GLFW_PRESS)
	{
		graphicDriver->technique = GraphicDriver::TECHNIQUE::AVERAGE_Y;
		std::cout << "COLOR TECHNIQUE SET TO: AVERAGE VERTICAL POSITION" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_3) == GLFW_PRESS)
	{
		graphicDriver->technique = GraphicDriver::TECHNIQUE::AVERAGE_Y_BALANCE;
		std::cout << "COLOR TECHNIQUE SET TO: VERTICAL BALANCE OF TRIANGLE" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_4) == GLFW_PRESS)
	{
		graphicDriver->technique = GraphicDriver::TECHNIQUE::AREA;
		std::cout << "COLOR TECHNIQUE SET TO: AREA OF TRIANGLE" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_5) == GLFW_PRESS)
	{
		graphicDriver->technique = GraphicDriver::TECHNIQUE::RANDOM;
		std::cout << "COLOR TECHNIQUE SET TO: RANDOM" << std::endl;
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_KP_0) == GLFW_PRESS)
	{
		graphicDriver->invertColor = !graphicDriver->invertColor;
		std::cout << "INVERTED THE COLOR" << std::endl;
		return true;
	}	
	// DEBUGGING
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_F3) == GLFW_PRESS)
	{
		int i = 0;
		std::cout << "PRINTING TRIANGLES' COORDINATES" << std::endl;
		for (Triangle* triangle : triangulation->triangulation)
		{
			std::cout << i++ << "\t" << triangle->Print() << std::endl;
		}
		return true;
	}
	if (glfwGetKey(graphicDriver->window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		std::cout << "CURRENT CONF AVERAGE FPS: " << 1.f / deltaTime << std::endl;
		return true;
	}
}