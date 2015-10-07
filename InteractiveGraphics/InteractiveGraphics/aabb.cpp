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

void AABB::draw3DSegment(
	const V3 &v0, 
	const V3 &c0, 
	const V3 &v1, 
	const V3 &c1,
	FrameBuffer &fb,
	const PPC &ppc) const {

	V3 projv0, projv1;
	if (!ppc.project(v0, projv0))
		return;
	if (!ppc.project(v1, projv1))
		return;
	fb.draw2DSegment(projv0, c0, projv1, c1);

}

void AABB::draw(
	FrameBuffer & fb, 
	const PPC & ppc, 
	unsigned int colorNear, 
	unsigned int colorFar) const
{
	V3 cnear;
	cnear.setFromColor(colorNear);
	V3 cfar;
	cfar.setFromColor(colorFar);

	V3 p0, p1;

	// Make copy in order to be able to use [] operators 
	// while honoring the const signature
	V3 corner1 = corners[0];
	V3 corner2 = corners[1];

	p0 = V3(corner1[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner1[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner1[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner2[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner2[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner1[0], corner2[1], corner1[2]);
	p1 = V3(corner1[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner1[2]);
	p1 = V3(corner2[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner1[2]);
	p1 = V3(corner2[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner1[2]);
	p1 = V3(corner1[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner2[2]);
	p1 = V3(corner1[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner2[2]);
	p1 = V3(corner2[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

#if 0
	4		7

		0------ - 3
		| 5 | 6
		| |
		1------ - 2
#endif
}
