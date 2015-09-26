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

	void scale(float scaleFactor);
	void translate(const V3 &translationVector);
	// rotate about axis
	void rotateAboutAxis(const V3 &aO, const V3 &adir, float theta);
};