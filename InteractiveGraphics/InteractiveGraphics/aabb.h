#pragma once

#include "v3.h"

// axis aligned bounding box small class
class AABB {
private:
	V3 corners[2];
public:
	AABB(const V3 &firstPoint);

	void AddPoint(const V3 &newPoint);
	V3 getFristCorner(void) const { return corners[0]; }
	V3 getSecondCorner(void) const { return corners[1]; }
};