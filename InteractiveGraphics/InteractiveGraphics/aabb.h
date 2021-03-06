#pragma once

#include "v3.h"
#include "sw_framebuffer.h"
#include "ppc.h"

// axis aligned bounding box small class
class AABB {
private:
	V3 corners[2];

	void draw3DSegment(
		const V3 &v0,
		const V3 &c0,
		const V3 &v1,
		const V3 &c1,
		SWFrameBuffer &fb,
		const PPC &ppc) const;
public:
	AABB(const V3 &firstPoint);

	void AddPoint(const V3 &newPoint);
	V3 getFristCorner(void) const { return corners[0]; }
	V3 getSecondCorner(void) const { return corners[1]; }

	void scale(float scaleFactor);
	void translate(const V3 &translationVector);

	bool clipWithFrame(float left, float top, float right, float bottom);
	void setPixelRectangle(int& left, int& right, int& top, int& bottom);

	void draw(
		SWFrameBuffer & fb,
		const PPC & ppc, 
		unsigned int colorNear, 
		unsigned int colorFar) const;
};