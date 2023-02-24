#pragma once

#include "Color.h"
#include "Intersection.h"

class RaytraceResult
{
public:
	Color color;
	IntersectionType intersecter;

	RaytraceResult(Color c, IntersectionType i) {
		this->color = c;
		this->intersecter = i;
	}
};

