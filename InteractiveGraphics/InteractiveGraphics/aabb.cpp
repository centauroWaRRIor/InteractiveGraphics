#include "aabb.h"


AABB::AABB(const V3 &firstPoint) {

	corners[0] = firstPoint;
	corners[1] = firstPoint;
}

void AABB::AddPoint(const V3 &newPoint) {

	V3 newPointCopy(newPoint); // honor const
	for (int i = 0; i < 3; i++) {
		if (newPointCopy[i] > corners[1][i])
			corners[1][i] = newPointCopy[i];
		if (newPointCopy[i] < corners[0][i])
			corners[0][i] = newPointCopy[i];
	}
}
