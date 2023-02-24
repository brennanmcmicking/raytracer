#pragma once

#include "Vec.h"
#include "Sphere.h"

enum IntersectionType {
	NONE = 0,
	SPHERE,
};

class Intersection
{
public:
	Vec point;
	Vec normal;
	Sphere* sphere;
	IntersectionType intersecter;

	Intersection(Vec p, Vec n, Sphere* s, IntersectionType i) {
		point = p;
		normal = n;
		sphere = s;
		intersecter = i;
	}
};

