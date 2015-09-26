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

void AABB::scale(float scaleFactor)
{
	corners[0] = corners[0] * scaleFactor;
	corners[1] = corners[1] * scaleFactor;
}

void AABB::translate(const V3 & translationVector)
{
	corners[0] = corners[0] + translationVector;
	corners[1] = corners[1] + translationVector;
}

void AABB::rotateAboutAxis(const V3 & aO, const V3 & adir, float theta)
{
	corners[0].rotateThisPointAboutAxis(aO, adir, theta);
	corners[1].rotateThisPointAboutAxis(aO, adir, theta);
}
