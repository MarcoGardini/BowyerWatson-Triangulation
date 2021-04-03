#pragma once

#define TRIANGULATION_MAX_POINTS 5000
#define TRIANGULATION_MIN_POINTS 0 
#define MS_TO_REPEAT_KEY_PRESS 200 
#define MAX_TRIANGLES TRIANGULATION_MAX_POINTS * 3

#include "Graphics.hpp"
#include "Triangulation.hpp"


class TriangulationManager
{
	GraphicDriver* graphicDriver;	
	Triangulation* triangulation;

public:
	double deltaTime; // Time between frames
	float pointSpeed; // Speed of point movement

	// constructor
	TriangulationManager();

	// contains main loop
	void Run();

	// input processing, check Controls.txt for list
	bool ProcessInput();
};