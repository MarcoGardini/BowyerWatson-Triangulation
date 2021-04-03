#pragma once

#include <vector>
#include <array>
#include <forward_list>
#include <unordered_set>
#include <random>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <glm/glm/gtx/norm.hpp>

static std::random_device device;
static std::mt19937 generator(device());
static std::uniform_real_distribution<float> distribution; 

class Point
{
public:
	glm::vec2 position;
	glm::vec2 direction;
	float speed;

	Point(glm::vec2 pos, glm::vec2 dir, float spd = 0) :
		position(pos), direction(dir), speed(spd)
	{}

	void Update(float deltaTime, float playground)
	{
		// change point direction once every 2k frames
		if (distribution(generator) > playground * .999f)
			direction = glm::normalize(glm::vec2(distribution(generator), distribution(generator)));

		// check if point is outside playground
		glm::vec2 newPos = position + direction * speed * deltaTime;
		if (glm::length2(newPos) > playground * playground)
		{
			direction = -direction;
			position += direction * speed * deltaTime;
		}
		else
			position = newPos;
	}
};

class Edge
{
public:
	glm::vec2 A, B;

	Edge(glm::vec2 a, glm::vec2 b) : A(a), B(b) {}

	bool Compare(Edge* edge)
	{
		return A == edge->A && B == edge->B || A == edge->B && B == edge->A;
	}
};

class Triangle
{
private:
	
	// sorting counter-clockwise
	void SortCCW()
	{
		// find highest
		if (pointA.y < pointB.y) std::swap(pointA, pointB);
		if (pointA.y < pointC.y) std::swap(pointA, pointC);

		// find "leftest"
		if (pointB.x > pointC.y) std::swap(pointB, pointC);
	}

	// calculate center and radius of the circumcircle
	void CalculateCircumscribedCircle()
	{
		/* the center of the circumscribed circle is the interception of the perpendicular bisectors.
		** we find the perpendicular bisector of 2 sides in the form y = mx + q, then solve the 
		&& system of equations */

		// mid point of 2 sides
		glm::vec2 midAB((pointA.x + pointB.x) / 2.f, (pointA.y + pointB.y) / 2.f);
		glm::vec2 midAC((pointA.x + pointC.x) / 2.f, (pointA.y + pointC.y) / 2.f);
		// slope (m) of the 2 sides (the perpendicular m is the negative reciprocal)
		float slopeAB = (pointB.y - pointA.y) / (pointB.x - pointA.x);
		float slopeAC = (pointC.y - pointA.y) / (pointC.x - pointA.x);
		// check degenerate cases, where the perpendicular is vertical (infinite m)
		if (slopeAB == 0.f)
		{
			circumCenter.x = midAB.x; 
			float intercept = midAC.y - midAC.x * (-1 / slopeAC); // negative reciprocal
			circumCenter.y = (-1 / slopeAC) * circumCenter.x + intercept;
		}
		else if (slopeAC == 0.f)
		{
			circumCenter.x = midAC.x;
			float intercept = midAB.y + midAB.x / slopeAB;
			circumCenter.y = circumCenter.x / -slopeAB + intercept;
		}
		else
		{
			// find the "q"s, always using the negative reciprocal of m
			float interceptAB = midAB.y + midAB.x / slopeAB;
			float interceptAC = midAC.y + midAC.x / slopeAC;
			// solve the system
			circumCenter.x = (interceptAC - interceptAB) / (-1 / slopeAB + 1 / slopeAC);
			circumCenter.y = -circumCenter.x / slopeAB + interceptAB;
		}
		// find the radius via a simple vector operation
		circumRadius = glm::length(pointA - circumCenter);		
	}

public:
	glm::vec2 pointA, pointB, pointC;
	std::array<Edge*, 3>	 edges;
	std::array<Triangle*, 3> links;
	float	  circumRadius;
	glm::vec2 circumCenter;
	int poolIndex;

	// constructor
	Triangle(int index)
	{
		poolIndex = index;		
		// just create the Edges, will be populated later 
		edges[0] = new Edge(pointA, pointB);
		edges[1] = new Edge(pointB, pointC);
		edges[2] = new Edge(pointC, pointA);		
	}

	// set points and edges
	void Populate(glm::vec2 A, glm::vec2 B, glm::vec2 C)
	{
		pointA.x = A.x; pointA.y = A.y;
		pointB.x = B.x; pointB.y = B.y;
		pointC.x = C.x; pointC.y = C.y;

		SortCCW();

		CalculateCircumscribedCircle();

		edges[0]->A = pointA; edges[0]->B = pointB;
		edges[1]->A = pointB; edges[1]->B = pointC;
		edges[2]->A = pointC; edges[2]->B = pointA;

		links[0] = nullptr; links[1] = nullptr; links[2] = nullptr;
	}

	// is point distance from circumcenter higher than circumradius
	bool isPointInsideCircumcircle(glm::vec2 point)
	{
		return glm::distance2(point, circumCenter) < circumRadius * circumRadius;
	}

	// debugging purpouses
	std::string Print() {
		return std::string("{[" + std::to_string(pointA.x) + "," + std::to_string(pointA.y) + "][" +
			std::to_string(pointB.x) + "," + std::to_string(pointB.y) + "][" +
			std::to_string(pointC.x) + "," + std::to_string(pointC.y) + "]}");
	}
};

class TrianglePool
{	
private:	
	// avoid fragmentation by saving inactive triangles in a linked list
	std::vector<Triangle*>  trianglePool;
	std::forward_list<int> inactiveTrianglesIndices;

public:
	int currentSize;
	
	// constructor
	TrianglePool(int maxTriangles)
	{
		currentSize = 0;
		for (int i = 0; i < maxTriangles; i++)
			trianglePool.push_back(new Triangle(i));		
	}

	// returns first inactive triangle, or first new triangle
	Triangle* Get()
	{
		Triangle* thisTriangle;
		if (!inactiveTrianglesIndices.empty())
		{
			thisTriangle = trianglePool[inactiveTrianglesIndices.front()];
			inactiveTrianglesIndices.pop_front();
		}
		else
		{
			thisTriangle = trianglePool[currentSize];			
			currentSize++;
		}
		return thisTriangle;
	}

	// remove triangle by adding it to the inactive list
	void Remove(Triangle* triangle)
	{
		inactiveTrianglesIndices.push_front(triangle->poolIndex);
	}

	// clean inactives and set size to zero
	void Clear()
	{
		inactiveTrianglesIndices.clear();
		currentSize = 0;
	}
};

class Triangulation
{
private:
	float scale;
	float playground; // points can move and exist only inside the playground, which is slightly bigger than screen
	std::vector<Point*> points;
	TrianglePool* trianglePool;
	
	// recursive function to check contiguous if contiguous triangles contain the new point in their circumcircle
	void CheckEdges(std::unordered_set<Triangle*>* _badTriangles, glm::vec2 point, Triangle* triangle)
	{
		for (int edge = 0; edge < 3; edge++)
		{
			if (triangle->links[edge] != nullptr &&
				triangle->links[edge]->isPointInsideCircumcircle(point) &&
				_badTriangles->find(triangle->links[edge]) == _badTriangles->end())
			{
				_badTriangles->insert(triangle->links[edge]);
				CheckEdges(_badTriangles, point, triangle->links[edge]);
			}
		}
	}
	
	// find if triangle1 shares one edge with triangle2, and link them
	bool LinkTriangles(Triangle* triangle1, Triangle* triangle2)
	{
		if (triangle1 != triangle2 && triangle1 != nullptr && triangle2 != nullptr)
		{
			for (int outer = 0; outer < 3; outer++)
			{
				for (int inner = 0; inner < 3; inner++)
				{
					if (triangle1->edges[outer]->Compare(triangle2->edges[inner]))
					{
						triangle1->links[outer] = triangle2;
						return true;
					}
				}
			}
		}
		return false;
	}

	/* loop through each point to add, brute force search through each triangle, if
	*  the point is contained in the triangle's circumcircle remove it. Create a polygon with
	*  each removed triangle's outer edge, and add the triangles made by linking each edge to
	*  the new point
	*/
	void Triangulate()
	{
		triangulation.clear();
		trianglePool->Clear();
		// first triangle, containing the full playground
		Triangle* superTriangle = trianglePool->Get();
		superTriangle->Populate(glm::vec2(-scale, -scale),
			glm::vec2(scale, -scale),
			glm::vec2(0.f, scale * scale));
		triangulation.insert(superTriangle);

		std::vector<Triangle*> badTriangles;
		std::vector<Edge> polygon;

		for (int i = 0; i < howManyPoints; i++)
		{
			glm::vec2 point = points[i]->position;

			badTriangles.clear();
			polygon.clear();
			
			// check if triangle contains point in its circumcircle
			for (Triangle* triangle : triangulation)
			{
				if (triangle->isPointInsideCircumcircle(point))
					badTriangles.push_back(triangle);
			}

			// create the outer polygon 
			for (int outer = 0; outer < badTriangles.size(); outer++)
			{
				for (int edge = 0; edge < 3; edge++)
				{
					bool isShared = false;
					for (int inner = 0; inner < badTriangles.size(); inner++)
					{
						if (inner != outer && !isShared)
						{
							for (int badEdge = 0; badEdge < 3; badEdge++)
							{
								if (badTriangles[outer]->edges[edge]->Compare(badTriangles[inner]->edges[badEdge]))
								{
									isShared = true;
								}
							}
						}
					}
					if (!isShared)
						polygon.push_back(*badTriangles[outer]->edges[edge]);
				}
			}

			// remove bad triangles
			for (int j = 0; j < badTriangles.size(); j++)
			{
				trianglePool->Remove(badTriangles[j]);
				triangulation.erase(badTriangles[j]);
			}

			// create new triangles
			for (int j = 0; j < polygon.size(); j++)
			{
				Triangle* thisTriangle = trianglePool->Get();
				thisTriangle->Populate(polygon[j].A, polygon[j].B, point);
				triangulation.insert(thisTriangle);
			}
		}
	}

	/* Smarter implementation, based on the assumption that only contiguous triangles can contain
	*  the point in their circumcircle. Main differences are 1) as we find the first "bad" triangle, 
	*  we just check the contiguous ones 2) we need to keep the triangles linked together
	*/
	void ContiguousTriangulate()
	{
		triangulation.clear();
		trianglePool->Clear();
		Triangle* superTriangle = trianglePool->Get();
		superTriangle->Populate(glm::vec2(-scale, -scale),
			glm::vec2(scale, -scale),
			glm::vec2(0.f, scale * scale));

		triangulation.insert(superTriangle);

		std::vector<Edge> polygon;
		std::unordered_set<Triangle*> badTriangles;
		std::unordered_set<Triangle*> outerTriangles;
		std::unordered_set<Triangle*> partialTriangulation;
		
		for (int i = 0; i < howManyPoints; i++)
		{
			glm::vec2 point = points[i]->position;

			polygon.clear();
			badTriangles.clear();
			outerTriangles.clear();
			partialTriangulation.clear();
			
			for (Triangle* triangle : triangulation)
			{
				if (triangle->isPointInsideCircumcircle(point))
				{
					badTriangles.insert(triangle);
					CheckEdges(&badTriangles, point, triangle);
					break;
				}
			}
						
			for (Triangle* badTriangle : badTriangles)
			{
				for (int edge = 0; edge < 3; edge++)
				{
					if (badTriangles.find(badTriangle->links[edge]) == badTriangles.end())
					{
						polygon.push_back(*badTriangle->edges[edge]);
						// save the link with outer triangles
						if (badTriangle->links[edge] != nullptr &&
							badTriangles.find(badTriangle->links[edge]) == badTriangles.end())
							outerTriangles.insert(badTriangle->links[edge]);
					}
				}
			}

			for (Triangle* badTriangle : badTriangles)
			{
				trianglePool->Remove(badTriangle);
				triangulation.erase(badTriangle);
			}

			// insert new triangles into a "partial" triangulation, allowing for linking
			for (int j = 0; j < polygon.size(); j++)
			{
				Triangle* thisTriangle = trianglePool->Get();
				thisTriangle->Populate(polygon[j].A, polygon[j].B, point);
				partialTriangulation.insert(thisTriangle);
			}

			for (Triangle* partialTriangle : partialTriangulation)
			{
				// link with outer triangles
				for (Triangle* outerTriangle : outerTriangles)
				{
					if (LinkTriangles(partialTriangle, outerTriangle))
					{
						LinkTriangles(outerTriangle, partialTriangle);
						break;
					}
				}

				// link "partial" triangles with each other
				int maxTwoLinked = 0;
				for (Triangle* otherPartialTriangle : partialTriangulation)
				{
					if (LinkTriangles(partialTriangle, otherPartialTriangle))
						maxTwoLinked++;
					if (maxTwoLinked == 2) break;
				}

				triangulation.insert(partialTriangle);
			}
		}
	}

public:
	enum class ALGORITHM	
	{
		BowyerWatson,
		ContiguousBowyerWatson,
		MAX_ALGORYTHM,
	};

	int howManyPoints;
	ALGORITHM algorithm;
	std::unordered_set<Triangle*> triangulation;
	
	// constructor
	Triangulation(float pointSpeed, int maxTriangles)
	{
		howManyPoints = 100;
		trianglePool = new TrianglePool(maxTriangles);
		scale = 3.0f;
		playground = 1.3f;
		distribution = std::uniform_real_distribution<float>(-playground, playground);
		GeneratePoints(pointSpeed);
		Triangulate();
	}

	// move each point and triangulate
	void Update(float deltaTime)
	{
		for (Point* point : points)
			point->Update(deltaTime, playground);		
		switch (algorithm)
		{
		case ALGORITHM::BowyerWatson:
			Triangulate();
			break;
		case ALGORITHM::ContiguousBowyerWatson:
			ContiguousTriangulate();
			break;
		}
	}

	// create new random points
	void GeneratePoints(float speed)
	{		
		// cleanup points
		for (auto p : points)
			delete p;
		points.clear();

		// ..and recreate them
		for (int i = 0; i < howManyPoints; i++)
		{
			points.push_back(new Point(glm::vec2(distribution(generator), distribution(generator)),
									   glm::normalize(glm::vec2(distribution(generator), distribution(generator))),
									   speed));
		}
	}	
};