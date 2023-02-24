#pragma once

#include <vector>

#include "Sphere.h"
#include "Light.h"
#include "Color.h"

class World
{
public:
	Sphere* spheres[15];
	int numSpheres = 0;
	Light* lights[10];
	int numLights = 0;
	Color background;
	Color ambient;
};

