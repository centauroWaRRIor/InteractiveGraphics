#include "framebuffer.h"
#include "scene.h"
#include "lodepng.h"
#include "scene.h"
#include <iostream>
#include <math.h>
#include <cfloat> // delete me when no longer using FLT_MAX
#include <vector>
using std::vector;

using namespace std;

// makes an OpenGL window that supports SW, HW rendering, that can be displayed on screen
//        and that receives UI events, i.e. keyboard, mouse, etc.
FrameBuffer::FrameBuffer(int u0, int v0, 
  unsigned int _w, unsigned int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {

  w = _w;
  h = _h;
  pix = new unsigned int[w*h];
  zb = new float[w*h];
}

FrameBuffer::~FrameBuffer()
{
	delete[] pix;
	delete[] zb;
}

// rendering callback; see header file comment
void FrameBuffer::draw() {

	// SW window, just transfer computed pixels from pix to HW for display
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);

}


// function called automatically on event within window (callback)
int FrameBuffer::handle(int event)  {  

  switch(event) 
  {   
  case FL_KEYBOARD: {
    KeyboardHandle();
    return 0;
  }
  default:
    break;
  }
  return 0;

}


void FrameBuffer::KeyboardHandle() {

	float tiltAmount = 1.0f;
	float rollAmount = 1.0f;
	float panAmount = 1.0f;
	float moveRightAmount = 5.0;//0.5f;
	float moveUpAmount = 5.0;//0.5f;
	float moveForwardAmount = 5.0;//0.5f;
	float zoomFactor = 0.99f;
	int key = Fl::event_key();
	if (scene) {
		switch (key) {
		case FL_Control_L: // avoids printing an error message when this is pressed
		case FL_Left:
		case FL_Right:
			// if ctrl + left or ctrl + right
			if (Fl::get_key(FL_Control_L) && 
				(key == FL_Left || key == FL_Right)) {
				rollAmount = (key == FL_Left) ? rollAmount : -1.0f * rollAmount;
				scene->getCamera()->roll(rollAmount);
				scene->currentSceneRedraw();
			}
			// if only left or only right
			else if ((key == FL_Left || key == FL_Right)) {
				panAmount = (key == FL_Left) ? panAmount : -1.0f * panAmount;
				scene->getCamera()->pan(panAmount);
				scene->currentSceneRedraw();
				//cerr << "pressed left arrow" << endl;
			}
			break;
		case FL_Up:
		case FL_Down:
			tiltAmount = (key == FL_Up) ? tiltAmount : -1.0f * tiltAmount;
			scene->getCamera()->tilt(tiltAmount);
			scene->currentSceneRedraw();
			break;
		case 'd':
		case 'a':
			moveRightAmount = (key == 'd') ? moveRightAmount : -1.0f * moveRightAmount;
			scene->getCamera()->moveRight(moveRightAmount);
			scene->currentSceneRedraw();
			//cerr << "Eyepoint: " << scene->getCamera()->getEyePoint() << endl;
			//cerr << "pressed a" << endl;
			break;
		case 'w':
		case 's':
			moveForwardAmount = (key == 'w') ? moveForwardAmount : -1.0f * moveForwardAmount;
			scene->getCamera()->moveForward(moveForwardAmount);
			scene->currentSceneRedraw();
			break;
		case 'q':
		case 'e':
			moveUpAmount = (key == 'q') ? moveUpAmount : -1.0f * moveUpAmount;
			scene->getCamera()->moveUp(moveUpAmount);
			scene->currentSceneRedraw();
			break;
		case 'z':
		case 'x':
			zoomFactor = (key == 'z') ? 1 + (1 - zoomFactor) : zoomFactor;
			scene->getCamera()->zoom(zoomFactor);
			scene->currentSceneRedraw();
			break;

		default:
			cerr << "INFO: do not understand keypress" << endl;
		}
	}
}

// clear to background color
void FrameBuffer::set(unsigned int color) {

  for (int i = 0; i < w*h; i++) {
    pix[i] = color;
  }

}

void FrameBuffer::clearZB(float farz)
{
	for (int i = 0; i < w*h; i++) {
		zb[i] = farz;
	}
}

// set pixel with coordinates u v to color provided as parameter
void FrameBuffer::setSafe(int u, int v, unsigned int color) {

  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return;

  set(u, v, color);

}

void FrameBuffer::set(int u, int v, unsigned int color) {

  pix[(h-1-v)*w+u] = color;

}

// set to checkboard
void FrameBuffer::setCheckerboard(int checkerSize, unsigned int color0, 
  unsigned int color1) {

  for (int v = 0; v < h; v++) {
    for (int u = 0; u < w; u++) {
      int cu = u / checkerSize;
      int cv = v / checkerSize;
      if (((cu + cv) % 2) == 0) {
        set(u, v, color0);
      }
      else {
        set(u, v, color1);
      }
    }
  }

}

// sets pixel at p[0], p[1] to color c if and only if 
// p[2] is closer than zb value at that pixel
void FrameBuffer::setIfOneOverWCloser(const V3 & p, const V3 & c)
{
	if ((p.getX() < 0.0f) || (p.getX() >= w) || 
		(p.getY() < 0.0f) || (p.getY() >= h))
		return;

	int u = (int)p.getX();
	int v = (int)p.getY();

	// remember that the z component of the projected point is 1/z or 1/w
	if (zb[(h - 1 - v)*w + u] >= p.getZ())
		return; // nothing to draw, already saw a surface closer at that pixel

	zb[(h - 1 - v)*w + u] = p.getZ(); // set z at pixel to new closest surface value
	set(u, v, c.getColor());
}

void FrameBuffer::setIfWCloser(const V3 & p, const V3 & c)
{
	if ((p.getX() < 0.0f) || (p.getX() >= w) ||
		(p.getY() < 0.0f) || (p.getY() >= h))
		return;

	int u = (int)p.getX();
	int v = (int)p.getY();

	// remember that here we are assuming the z component of the point passed is
	// w instead of 1/w. This is produced by the perspective correct linear interpolation
	// due to 1/w not being linear in model space but in screen space and w not being 
	// linear in screen space but in model space
	if (zb[(h - 1 - v)*w + u] <= p.getZ())
		return; // nothing to draw, already saw a surface closer at that pixel

	zb[(h - 1 - v)*w + u] = p.getZ(); // set z at pixel to new closest surface value
	set(u, v, c.getColor());
}

// draw circle
void FrameBuffer::draw2DCircle(float cuf, float cvf, float radius, 
  unsigned int color)
{

    // axis aligned bounding box (AABB) of circle (it's a square parallel to axes)
    int top = (int) (cvf - radius + 0.5f);
    int bottom = (int) (cvf + radius - 0.5f);
    int left = (int) (cuf - radius + 0.5f);
    int right = (int) (cuf + radius - 0.5f);

    // clip AABB with frame
    if (top > h-1 || bottom < 0 || right < 0 || left > w-1)
      return;
    if (left < 0)
      left = 0;
    if (right > w-1)
      right = w-1;
    if (top < 0)
      top = 0;
    if (bottom > h-1)
      bottom = h-1;

    float radius2 = radius*radius;
    for (int v = top; v <= bottom; v++) {
      for (int u = left; u <= right; u++) {
        float uf = .5f + (float)u;
        float vf = .5f + (float)v;
        float d2 = (cvf-vf)*(cvf-vf)+(cuf-uf)*(cuf-uf);
        if (d2 > radius2)
          continue;
        set(u, v, color); // ignores z-buffer
      }
    }
}

// draw 2D circle only where it wins the z-fight
void FrameBuffer::draw2DCircleIfCloser(const V3 &p, float radius, const V3 &color)
{

	// axis aligned bounding box (AABB) of circle (it's a square parallel to axes)
	int top = (int)(p.getY() - radius + 0.5f);
	int bottom = (int)(p.getY() + radius - 0.5f);
	int left = (int)(p.getX() - radius + 0.5f);
	int right = (int)(p.getX() + radius - 0.5f);

	// clip AABB with frame
	if (top > h - 1 || bottom < 0 || right < 0 || left > w - 1)
		return;
	if (left < 0)
		left = 0;
	if (right > w - 1)
		right = w - 1;
	if (top < 0)
		top = 0;
	if (bottom > h - 1)
		bottom = h - 1;

	float radius2 = radius*radius;
	for (int v = top; v <= bottom; v++) {
		for (int u = left; u <= right; u++) {
			float uf = .5f + (float)u;
			float vf = .5f + (float)v;
			float d2 = (p.getY() - vf)*(p.getY() - vf) + 
				(p.getX() - uf)*(p.getX() - uf);
			if (d2 > radius2)
				continue;
			setIfOneOverWCloser(V3(uf,vf, p.getZ()), color);
		}
	}
}

// draw axis aligned rectangle
void FrameBuffer::draw2DRectangle(
	float llu, 
	float llv, 
	float width, 
	float height,		
	unsigned int color)
{

	// axis aligned bounding box (AABB) of circle (it's a square parallel to axes)
	int top = (int)(llv - height + 0.5f);
	int bottom = (int)(llv - 0.5f);
	int left = (int)(llu + 0.5f);
	int right = (int)(llu + width - 0.5f);

	// clip AABB with frame
	if (top > (h - 1) || bottom < 0 || right < 0 || (left > w - 1) )
		return;
	if (left < 0)
		left = 0;
	if (right > w - 1)
		right = w - 1;
	if (top < 0)
		top = 0;
	if (bottom > h - 1)
		bottom = h - 1;

	for (int v = top; v <= bottom; v++) {
		for (int u = left; u <= right; u++) {
			float uf = .5f + (float)u;
			float vf = .5f + (float)v;
			set(u, v, color);
		}
	}
}

void FrameBuffer::draw2DFlatTriangle(V3 *const pvs, unsigned int color)
{
	// compute screen axes-aligned bounding box for triangle 
	// clipping against framebuffer
	AABB aabb(pvs[0]);
	aabb.AddPoint(pvs[1]);
	aabb.AddPoint(pvs[2]);

	if (!aabb.clipWithFrame(0.0f, 0.0f, (float)w, (float)h))
		return;

	int left, right, top, bottom;
	aabb.setPixelRectangle(left, right, top, bottom);

	// set edge equations
	V3 eeqs[3]; // eeqs[0] = (A, B, C), where Au + Bv + C
	for (int ei = 0; ei < 3; ei++) {
		int e1 = (ei + 1) % 3;
		eeqs[ei][0] = pvs[e1][1] - pvs[ei][1];
		eeqs[ei][1] = pvs[ei][0] - pvs[e1][0];
		eeqs[ei][2] = -pvs[ei][1] * eeqs[ei][1] - pvs[ei][0] * eeqs[ei][0];
		int e2 = (e1 + 1) % 3;
		// plug third vertex into edge equation to establish
		// correct sidedness
		V3 pv3(pvs[e2][0], pvs[e2][1], 1.0f); // (u2, v2, 1)
		if (eeqs[ei] * pv3 < 0.0f)
			eeqs[ei] = eeqs[ei] * -1.0f;
	}
#if 0
	(v-v1) = (v2 - v1) (u - u1) 
		     ---------
		     (u2 - u1)

	(v - v1) (u2 - u1) = (v2 - v1) (u - u1)

	v (u2 - u1) - v1 (u2 - u1) = u (v2 - v1) - u1 (v2 - v1)

	u (v2 - v1) + v (u1 - u2) - u1 (v2 - v1) + v1 (u2 - u1) = 0

	u(v2 - v1) + v(u1 - u2) - u1(v2 - v1) - v1(u1 - u2) = 0

#endif
	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 

	for (currPixV = top,
		currEELS[0] = topLeftPixC * eeqs[0],
		currEELS[1] = topLeftPixC * eeqs[1],
		currEELS[2] = topLeftPixC * eeqs[2];
		currPixV <= bottom;
		currPixV++,
		currEELS[0] += eeqs[0][1] /*+=b[0]*/,
		currEELS[1] += eeqs[1][1] /*+=b[1]*/,
		currEELS[2] += eeqs[2][1] /*+=b[2]*/) {
		for (currPixU = left,
			currEE = currEELS;
			currPixU <= right;
			currPixU++,
			currEE[0] += eeqs[0][0] /*+=a[0]*/,
			currEE[1] += eeqs[1][0] /*+=a[1]*/,
			currEE[2] += eeqs[2][0] /*+=a[2]*/) {

			if (currEE[0] < 0 || currEE[1] < 0 || currEE[2] < 0) {
				continue; // outside triangle
			}
			else { // found pixel inside of triangle; set it to right color

				pixC = V3(.5f + (float)currPixU, .5f + (float)currPixV, 1.0f);
				set((int)pixC[0], (int)pixC[1], color); // ignores depth test
			}
		}
	}
}

void FrameBuffer::draw2DFlatTriangleScreenSpace(
	V3 *const pvs,
	V3 *const cols)
{
	// compute screen axes-aligned bounding box for triangle 
	// clipping against framebuffer
	AABB aabb(pvs[0]);
	aabb.AddPoint(pvs[1]);
	aabb.AddPoint(pvs[2]);

	if (!aabb.clipWithFrame(0.0f, 0.0f, (float)w, (float)h))
		return;

	int left, right, top, bottom;
	aabb.setPixelRectangle(left, right, top, bottom);

	// set edge equations
	V3 eeqs[3]; // eeqs[0] = (A, B, C), where Au + Bv + C
	for (int ei = 0; ei < 3; ei++) {
		int e1 = (ei + 1) % 3;
		eeqs[ei][0] = pvs[e1][1] - pvs[ei][1];
		eeqs[ei][1] = pvs[ei][0] - pvs[e1][0];
		eeqs[ei][2] = -pvs[ei][1] * eeqs[ei][1] - pvs[ei][0] * eeqs[ei][0];
		int e2 = (e1 + 1) % 3;
		// plug third vertex into edge equation to establish
		// correct sidedness
		V3 pv3(pvs[e2][0], pvs[e2][1], 1.0f); // (u2, v2, 1)
		if (eeqs[ei] * pv3 < 0.0f)
			eeqs[ei] = eeqs[ei] * -1.0f;
	}

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);
	// linear expressions for screen space interpolation of colors
	M33 colsABC;
	colsABC[0] = baryMatrixInverse*V3(cols[0][0], cols[1][0], cols[2][0]);
	colsABC[1] = baryMatrixInverse*V3(cols[0][1], cols[1][1], cols[2][1]);
	colsABC[2] = baryMatrixInverse*V3(cols[0][2], cols[1][2], cols[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result

	for (currPixV = top,
		currEELS[0] = topLeftPixC * eeqs[0],
		currEELS[1] = topLeftPixC * eeqs[1],
		currEELS[2] = topLeftPixC * eeqs[2];
		currPixV <= bottom;
		currPixV++,
		currEELS[0] += eeqs[0][1] /*+=b[0]*/ ,
		currEELS[1] += eeqs[1][1] /*+=b[1]*/ ,
		currEELS[2] += eeqs[2][1] /*+=b[2]*/) {
		for (currPixU = left,
			currEE = currEELS;
			currPixU <= right;
			currPixU++,
			currEE[0] += eeqs[0][0] /*+=a[0]*/,
			currEE[1] += eeqs[1][0] /*+=a[1]*/,
			currEE[2] += eeqs[2][0] /*+=a[2]*/) {

			if (currEE[0] < 0 || currEE[1] < 0 || currEE[2] < 0) {
				continue; // outside triangle
			}
			else { // found pixel inside of triangle; set it to right color

				pixC = V3(.5f + (float)currPixU, .5f + (float)currPixV, 1.0f);
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s
				interpolatedColor = colsABC*pixC; // color at current pixel interp. l s s

				//set(currPixX, currPixY, color); // ignores depth test
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DFlatTriangleModelSpace(
	const V3 & v1, const V3 & c1,
	const V3 & v2, const V3 & c2,
	const V3 & v3, const V3 & c3,
	M33 Q)
{
	float xCoords[3], yCoords[3]; // this format is more triangle edge equation friendly 
	xCoords[0] = v1.getX();
	xCoords[1] = v2.getX();
	xCoords[2] = v3.getX();
	yCoords[0] = v1.getY();
	yCoords[1] = v2.getY();
	yCoords[2] = v3.getY();

	// compute screen axes-aligned bounding box for triangle
	float bbox[2][2]; // for each x and y, store the min and max values
	bbox[0][0] = FLT_MAX;
	bbox[0][1] = FLT_MIN; // take into account that y is inverted in screen coordinates
	bbox[1][0] = FLT_MIN;
	bbox[1][1] = FLT_MAX; // take into account that y is inverted in screen coordinates
	for (int i = 0; i < 3; i++) {
		// find min x
		if (xCoords[i] < bbox[0][0])
			bbox[0][0] = xCoords[i];
		// find min y
		if (yCoords[i] > bbox[0][1])
			bbox[0][1] = yCoords[i];
		// find max x
		if (xCoords[i] > bbox[1][0])
			bbox[1][0] = xCoords[i];
		// find max y
		if (yCoords[i] < bbox[1][1])
			bbox[1][1] = yCoords[i];
	}

	// clip bounding box to screen boundaries
	if (bbox[1][1] > (h - 1) || bbox[0][1] < 0 || bbox[1][0] < 0 || (bbox[0][0] > w - 1))
		return; // discard this triangle
	if (bbox[0][0] < 0)
		bbox[0][0] = 0;
	if (bbox[1][0] > w - 1)
		bbox[1][0] = (float)w - 1;
	if (bbox[1][1] < 0)
		bbox[1][1] = 0;
	if (bbox[0][1] > h - 1)
		bbox[0][1] = (float)h - 1;

	// build rasterization parameters to be lerped in screen space
	V3 redParameters(c1.getX(), c2.getX(), c3.getX());
	V3 greenParameters(c1.getY(), c2.getY(), c3.getY());
	V3 blueParameters(c1.getZ(), c2.getZ(), c3.getZ());

	// w is linear in model space and not linear in screen space.For 1 / w, it's the other way around.
	// Hence we want to use w here. Projected z is coming out as 1/w by construction so we want to
	// invert that z to get back to original w.
	V3 wParameters(1/(v1.getZ()), 1/(v2.getZ()), 1/(v3.getZ()));

	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	float ARed = 
		Q[0][0] * redParameters[0] + 
		Q[1][0] * redParameters[1] + 
		Q[2][0] * redParameters[2];
	float AGreen = 
		Q[0][0] * greenParameters[0] +
		Q[1][0] * greenParameters[1] +
		Q[2][0] * greenParameters[2];
	float ABlue =
		Q[0][0] * blueParameters[0] +
		Q[1][0] * blueParameters[1] +
		Q[2][0] * blueParameters[2];
	float ADepth =
		Q[0][0] * wParameters[0] +
		Q[1][0] * wParameters[1] +
		Q[2][0] * wParameters[2];

	float BRed =
		Q[0][1] * redParameters[0] +
		Q[1][1] * redParameters[1] +
		Q[2][1] * redParameters[2];
	float BGreen =
		Q[0][1] * greenParameters[0] +
		Q[1][1] * greenParameters[1] +
		Q[2][1] * greenParameters[2];
	float BBlue =
		Q[0][1] * blueParameters[0] +
		Q[1][1] * blueParameters[1] +
		Q[2][1] * blueParameters[2];
	float BDepth =
		Q[0][1] * wParameters[0] +
		Q[1][1] * wParameters[1] +
		Q[2][1] * wParameters[2];

	float CRed = 
		Q[0][2] * redParameters[0] +
		Q[1][2] * redParameters[1] +
		Q[2][2] * redParameters[2];
	float CGreen =
		Q[0][2] * greenParameters[0] +
		Q[1][2] * greenParameters[1] +
		Q[2][2] * greenParameters[2];
	float CBlue =
		Q[0][2] * blueParameters[0] +
		Q[1][2] * blueParameters[1] +
		Q[2][2] * blueParameters[2];
	float CDepth =
		Q[0][2] * wParameters[0] +
		Q[1][2] * wParameters[1] +
		Q[2][2] * wParameters[2];

	float D = Q[0][0] + Q[1][0] + Q[2][0];
	float E = Q[0][1] + Q[1][1] + Q[2][1];
	float F = Q[0][2] + Q[1][2] + Q[2][2];

	// final raster parameter interpolated result stored here
	V3 interpolatedColor;
	float interpolatedZBufferDepth;

	float a[3], b[3], c[3]; // a,b,c for the three triangle edge expressions

	// establish the three edge equations 
	// edge that goes through vertices 0 and 1
	a[0] = yCoords[1] - yCoords[0];
	b[0] = -xCoords[1] + xCoords[0];
	c[0] = -xCoords[0] * yCoords[1] + yCoords[0] * xCoords[1];
	// edge that goes through vertices 1 and 2
	a[1] = yCoords[2] - yCoords[1];
	b[1] = -xCoords[2] + xCoords[1];
	c[1] = -xCoords[1] * yCoords[2] + yCoords[1] * xCoords[2];
	// edge that goes through vertices 2 and 0
	a[2] = yCoords[0] - yCoords[2];
	b[2] = -xCoords[0] + xCoords[2];
	c[2] = -xCoords[2] * yCoords[0] + yCoords[2] * xCoords[0];

	// temporary used to establish correct sidedness
	float sidedness;
	sidedness = a[0] * xCoords[2] + b[0] * yCoords[2] + c[0];
	// I need to investigate if this special case has to do with the
	// order in which the vertices are specified (i.e., cw vs ccw)
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[0] = -a[0];
		b[0] = -b[0];
		c[0] = -c[0];
	}
	sidedness = a[1] * xCoords[0] + b[1] * yCoords[0] + c[1];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[1] = -a[1];
		b[1] = -b[1];
		c[1] = -c[1];
	}
	sidedness = a[2] * xCoords[1] + b[2] * yCoords[1] + c[2];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[2] = -a[2];
		b[2] = -b[2];
		c[2] = -c[2];
	}

	// use clipped AABB
	int left = (int)(bbox[0][0] + 0.5);
	int right = (int)(bbox[1][0] - 0.5);
	int top = (int)(bbox[1][1] + 0.5);
	int bottom = (int)(bbox[0][1] - 0.5);

	int currPixX, currPixY; // current pixel considered
	float currEELS[3], currEE[3]; // edge expression values for the line starts and within line
	for (currPixY = top,
		currEELS[0] = a[0] * (left + 0.5f) + b[0] * (top + 0.5f) + c[0],
		currEELS[1] = a[1] * (left + 0.5f) + b[1] * (top + 0.5f) + c[1],
		currEELS[2] = a[2] * (left + 0.5f) + b[2] * (top + 0.5f) + c[2];
		currPixY <= bottom;
		currPixY++,
		currEELS[0] += b[0],
		currEELS[1] += b[1],
		currEELS[2] += b[2]) {
		for (currPixX = left,
			currEE[0] = currEELS[0],
			currEE[1] = currEELS[1],
			currEE[2] = currEELS[2];
			currPixX <= right;
			currPixX++,
			currEE[0] += a[0],
			currEE[1] += a[1],
			currEE[2] += a[2]) {

			if (currEE[0] < 0 || currEE[1] < 0 || currEE[2] < 0) {
				continue; // outside triangle
			}
			else { // found pixel inside of triangle; set it to interpolated color

				// find interpolated color by following the model space formula
				// for rater parameter linear interpolation 
				// r = ((A * u) + (B * v) + C) / ((D * u) + (E * v) + F)
				interpolatedColor[0] = 
					((ARed * currPixX) + (BRed * currPixY) + CRed) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[1] = 
					((AGreen * currPixX) + (BGreen * currPixY) + CGreen) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[2] = 
					((ABlue * currPixX) + (BBlue * currPixY) + CBlue) /
					((D * currPixX) + (E * currPixY) + F);

				interpolatedZBufferDepth = 
					((ADepth * currPixX) + (BDepth * currPixY) + CDepth) /
					((D * currPixX) + (E * currPixY) + F);

				setIfWCloser(V3((float)currPixX, (float)currPixY, interpolatedZBufferDepth), 
					interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DTexturedTriangle(
	const V3 & v1, const V3 & c1, 
	const V3 & v2, const V3 & c2, 
	const V3 & v3, const V3 & c3, 
	const V3 &sCoords, const V3 &tCoords,
	M33 baryMatrixInverse, 
	M33 perspCorrectMatQ,
	const Texture &texture)
{
	float xCoords[3], yCoords[3]; // this format is more triangle edge equation friendly 
	xCoords[0] = v1.getX();
	xCoords[1] = v2.getX();
	xCoords[2] = v3.getX();
	yCoords[0] = v1.getY();
	yCoords[1] = v2.getY();
	yCoords[2] = v3.getY();

	// compute screen axes-aligned bounding box for triangle
	float bbox[2][2]; // for each x and y, store the min and max values
	bbox[0][0] = FLT_MAX;
	bbox[0][1] = FLT_MIN; // take into account that y is inverted in screen coordinates
	bbox[1][0] = FLT_MIN;
	bbox[1][1] = FLT_MAX; // take into account that y is inverted in screen coordinates
	for (int i = 0; i < 3; i++) {
		// find min x
		if (xCoords[i] < bbox[0][0])
			bbox[0][0] = xCoords[i];
		// find min y
		if (yCoords[i] > bbox[0][1])
			bbox[0][1] = yCoords[i];
		// find max x
		if (xCoords[i] > bbox[1][0])
			bbox[1][0] = xCoords[i];
		// find max y
		if (yCoords[i] < bbox[1][1])
			bbox[1][1] = yCoords[i];
	}

	// clip bounding box to screen boundaries
	if (bbox[1][1] > (h - 1) || bbox[0][1] < 0 || bbox[1][0] < 0 || (bbox[0][0] > w - 1))
		return; // discard this triangle
	if (bbox[0][0] < 0)
		bbox[0][0] = 0;
	if (bbox[1][0] > w - 1)
		bbox[1][0] = (float)w - 1;
	if (bbox[1][1] < 0)
		bbox[1][1] = 0;
	if (bbox[0][1] > h - 1)
		bbox[0][1] = (float)h - 1;

	// make copy to honor const
	V3 sParameters(sCoords);
	V3 tParameters(tCoords);

	// 1/w is linear in screen space and gives greater precision for near 
	// objects when depth testing.
	V3 oneOverWParameters(v1.getZ(), v2.getZ(), v3.getZ());

	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients for 
	// model space interpolation
	float As =
		perspCorrectMatQ[0][0] * sParameters[0] +
		perspCorrectMatQ[1][0] * sParameters[1] +
		perspCorrectMatQ[2][0] * sParameters[2];
	float At =
		perspCorrectMatQ[0][0] * tParameters[0] +
		perspCorrectMatQ[1][0] * tParameters[1] +
		perspCorrectMatQ[2][0] * tParameters[2];

	float Bs =
		perspCorrectMatQ[0][1] * sParameters[0] +
		perspCorrectMatQ[1][1] * sParameters[1] +
		perspCorrectMatQ[2][1] * sParameters[2];
	float Bt =
		perspCorrectMatQ[0][1] * tParameters[0] +
		perspCorrectMatQ[1][1] * tParameters[1] +
		perspCorrectMatQ[2][1] * tParameters[2];

	float Cs =
		perspCorrectMatQ[0][2] * sParameters[0] +
		perspCorrectMatQ[1][2] * sParameters[1] +
		perspCorrectMatQ[2][2] * sParameters[2];
	float Ct =
		perspCorrectMatQ[0][2] * tParameters[0] +
		perspCorrectMatQ[1][2] * tParameters[1] +
		perspCorrectMatQ[2][2] * tParameters[2];

	// colors are available for future modes and for debug
	V3 redParameters(c1.getX(), c2.getX(), c3.getX());
	V3 greenParameters(c1.getY(), c2.getY(), c3.getY());
	V3 blueParameters(c1.getZ(), c2.getZ(), c3.getZ());

	float ARed =
		perspCorrectMatQ[0][0] * redParameters[0] +
		perspCorrectMatQ[1][0] * redParameters[1] +
		perspCorrectMatQ[2][0] * redParameters[2];
	float AGreen =
		perspCorrectMatQ[0][0] * greenParameters[0] +
		perspCorrectMatQ[1][0] * greenParameters[1] +
		perspCorrectMatQ[2][0] * greenParameters[2];
	float ABlue =
		perspCorrectMatQ[0][0] * blueParameters[0] +
		perspCorrectMatQ[1][0] * blueParameters[1] +
		perspCorrectMatQ[2][0] * blueParameters[2];

	float BRed =
		perspCorrectMatQ[0][1] * redParameters[0] +
		perspCorrectMatQ[1][1] * redParameters[1] +
		perspCorrectMatQ[2][1] * redParameters[2];
	float BGreen =
		perspCorrectMatQ[0][1] * greenParameters[0] +
		perspCorrectMatQ[1][1] * greenParameters[1] +
		perspCorrectMatQ[2][1] * greenParameters[2];
	float BBlue =
		perspCorrectMatQ[0][1] * blueParameters[0] +
		perspCorrectMatQ[1][1] * blueParameters[1] +
		perspCorrectMatQ[2][1] * blueParameters[2];

	float CRed =
		perspCorrectMatQ[0][2] * redParameters[0] +
		perspCorrectMatQ[1][2] * redParameters[1] +
		perspCorrectMatQ[2][2] * redParameters[2];
	float CGreen =
		perspCorrectMatQ[0][2] * greenParameters[0] +
		perspCorrectMatQ[1][2] * greenParameters[1] +
		perspCorrectMatQ[2][2] * greenParameters[2];
	float CBlue =
		perspCorrectMatQ[0][2] * blueParameters[0] +
		perspCorrectMatQ[1][2] * blueParameters[1] +
		perspCorrectMatQ[2][2] * blueParameters[2];

	float D = perspCorrectMatQ[0][0] + perspCorrectMatQ[1][0] + perspCorrectMatQ[2][0];
	float E = perspCorrectMatQ[0][1] + perspCorrectMatQ[1][1] + perspCorrectMatQ[2][1];
	float F = perspCorrectMatQ[0][2] + perspCorrectMatQ[1][2] + perspCorrectMatQ[2][2];

	// final raster parameter interpolated result stored here
	float interpolatedS, interpolatedT;
	float interpolatedZBufferDepth;
	V3 colorVector;
	unsigned int texelColor;

	// abc coefficients for the raster parameter interpolation
	// in screen space
	V3 abcDepth = baryMatrixInverse * oneOverWParameters;

	float a[3], b[3], c[3]; // a,b,c for the three triangle edge expressions

	// establish the three edge equations 
	// edge that goes through vertices 0 and 1
	a[0] = yCoords[1] - yCoords[0];
	b[0] = -xCoords[1] + xCoords[0];
	c[0] = -xCoords[0] * yCoords[1] + yCoords[0] * xCoords[1];
	// edge that goes through vertices 1 and 2
	a[1] = yCoords[2] - yCoords[1];
	b[1] = -xCoords[2] + xCoords[1];
	c[1] = -xCoords[1] * yCoords[2] + yCoords[1] * xCoords[2];
	// edge that goes through vertices 2 and 0
	a[2] = yCoords[0] - yCoords[2];
	b[2] = -xCoords[0] + xCoords[2];
	c[2] = -xCoords[2] * yCoords[0] + yCoords[2] * xCoords[0];

	// temporary used to establish correct sidedness
	float sidedness;
	sidedness = a[0] * xCoords[2] + b[0] * yCoords[2] + c[0];
	// I need to investigate if this special case has to do with the
	// order in which the vertices are specified (i.e., cw vs ccw)
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[0] = -a[0];
		b[0] = -b[0];
		c[0] = -c[0];
	}
	sidedness = a[1] * xCoords[0] + b[1] * yCoords[0] + c[1];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[1] = -a[1];
		b[1] = -b[1];
		c[1] = -c[1];
	}
	sidedness = a[2] * xCoords[1] + b[2] * yCoords[1] + c[2];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[2] = -a[2];
		b[2] = -b[2];
		c[2] = -c[2];
	}

	// use clipped AABB
	int left = (int)(bbox[0][0] + 0.5);
	int right = (int)(bbox[1][0] - 0.5);
	int top = (int)(bbox[1][1] + 0.5);
	int bottom = (int)(bbox[0][1] - 0.5);

	int currPixX, currPixY; // current pixel considered
	float currEELS[3], currEE[3]; // edge expression values for the line starts and within line
	for (currPixY = top,
		currEELS[0] = a[0] * (left + 0.5f) + b[0] * (top + 0.5f) + c[0],
		currEELS[1] = a[1] * (left + 0.5f) + b[1] * (top + 0.5f) + c[1],
		currEELS[2] = a[2] * (left + 0.5f) + b[2] * (top + 0.5f) + c[2];
		currPixY <= bottom;
		currPixY++,
		currEELS[0] += b[0],
		currEELS[1] += b[1],
		currEELS[2] += b[2]) {
		for (currPixX = left,
			currEE[0] = currEELS[0],
			currEE[1] = currEELS[1],
			currEE[2] = currEELS[2];
			currPixX <= right;
			currPixX++,
			currEE[0] += a[0],
			currEE[1] += a[1],
			currEE[2] += a[2]) {

			if (currEE[0] < 0 || currEE[1] < 0 || currEE[2] < 0) {
				continue; // outside triangle
			}
			else { // found pixel inside of triangle; set it to interpolated color

				// use model space linear interpolation for the s,t raster parameters
				interpolatedS =
					((As * currPixX) + (Bs * currPixY) + Cs) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedT =
					((At * currPixX) + (Bt * currPixY) + Ct) /
					((D * currPixX) + (E * currPixY) + F);

				// use model space linear interpolation for the r,g,b raster parameters
				V3 interpolatedColor;
				// find interpolated color by following the model space formula
				// for rater parameter linear interpolation 
				// r = ((A * u) + (B * v) + C) / ((D * u) + (E * v) + F)
				interpolatedColor[0] =
					((ARed * currPixX) + (BRed * currPixY) + CRed) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[1] =
					((AGreen * currPixX) + (BGreen * currPixY) + CGreen) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[2] =
					((ABlue * currPixX) + (BBlue * currPixY) + CBlue) /
					((D * currPixX) + (E * currPixY) + F);

				// use screen space linear interpolation for the depth since we are using 1/w
				interpolatedZBufferDepth = abcDepth[0] * currPixX + abcDepth[1] * currPixY + abcDepth[2];

				// sample texture using lerped result of s,t raster parameters (in model space)
				//texelColor = texture.sampleTexNearTile(interpolatedS, interpolatedT);
				texelColor = texture.sampleTexBilinearTile(interpolatedS, interpolatedT);
				colorVector.setFromColor(texelColor);

				// write to frame buffer if 1/w is closer works
				setIfOneOverWCloser(V3((float)currPixX, (float)currPixY, interpolatedZBufferDepth),
					colorVector);
			}
		}
	}
}

void FrameBuffer::draw2DSprite(
	const V3 & v1, const V3 & c1, 
	const V3 & v2, const V3 & c2, 
	const V3 & v3, const V3 & c3, 
	const V3 & sCoords, const V3 & tCoords, 
	M33 baryMatrixInverse, 
	M33 perspCorrectMatQ, 
	const Texture & texture)
{
	float xCoords[3], yCoords[3]; // this format is more triangle edge equation friendly 
	xCoords[0] = v1.getX();
	xCoords[1] = v2.getX();
	xCoords[2] = v3.getX();
	yCoords[0] = v1.getY();
	yCoords[1] = v2.getY();
	yCoords[2] = v3.getY();

	// compute screen axes-aligned bounding box for triangle
	float bbox[2][2]; // for each x and y, store the min and max values
	bbox[0][0] = FLT_MAX;
	bbox[0][1] = FLT_MIN; // take into account that y is inverted in screen coordinates
	bbox[1][0] = FLT_MIN;
	bbox[1][1] = FLT_MAX; // take into account that y is inverted in screen coordinates
	for (int i = 0; i < 3; i++) {
		// find min x
		if (xCoords[i] < bbox[0][0])
			bbox[0][0] = xCoords[i];
		// find min y
		if (yCoords[i] > bbox[0][1])
			bbox[0][1] = yCoords[i];
		// find max x
		if (xCoords[i] > bbox[1][0])
			bbox[1][0] = xCoords[i];
		// find max y
		if (yCoords[i] < bbox[1][1])
			bbox[1][1] = yCoords[i];
	}

	// clip bounding box to screen boundaries
	if (bbox[1][1] > (h - 1) || bbox[0][1] < 0 || bbox[1][0] < 0 || (bbox[0][0] > w - 1))
		return; // discard this triangle
	if (bbox[0][0] < 0)
		bbox[0][0] = 0;
	if (bbox[1][0] > w - 1)
		bbox[1][0] = (float)w - 1;
	if (bbox[1][1] < 0)
		bbox[1][1] = 0;
	if (bbox[0][1] > h - 1)
		bbox[0][1] = (float)h - 1;

	// make copy to honor const
	V3 sParameters(sCoords);
	V3 tParameters(tCoords);

	// 1/w is linear in screen space and gives greater precision for near 
	// objects when depth testing.
	V3 oneOverWParameters(v1.getZ(), v2.getZ(), v3.getZ());

	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients for 
	// model space interpolation
	float As =
		perspCorrectMatQ[0][0] * sParameters[0] +
		perspCorrectMatQ[1][0] * sParameters[1] +
		perspCorrectMatQ[2][0] * sParameters[2];
	float At =
		perspCorrectMatQ[0][0] * tParameters[0] +
		perspCorrectMatQ[1][0] * tParameters[1] +
		perspCorrectMatQ[2][0] * tParameters[2];

	float Bs =
		perspCorrectMatQ[0][1] * sParameters[0] +
		perspCorrectMatQ[1][1] * sParameters[1] +
		perspCorrectMatQ[2][1] * sParameters[2];
	float Bt =
		perspCorrectMatQ[0][1] * tParameters[0] +
		perspCorrectMatQ[1][1] * tParameters[1] +
		perspCorrectMatQ[2][1] * tParameters[2];

	float Cs =
		perspCorrectMatQ[0][2] * sParameters[0] +
		perspCorrectMatQ[1][2] * sParameters[1] +
		perspCorrectMatQ[2][2] * sParameters[2];
	float Ct =
		perspCorrectMatQ[0][2] * tParameters[0] +
		perspCorrectMatQ[1][2] * tParameters[1] +
		perspCorrectMatQ[2][2] * tParameters[2];

	// Lerp colors too
	V3 redParameters(c1.getX(), c2.getX(), c3.getX());
	V3 greenParameters(c1.getY(), c2.getY(), c3.getY());
	V3 blueParameters(c1.getZ(), c2.getZ(), c3.getZ());

	float ARed =
		perspCorrectMatQ[0][0] * redParameters[0] +
		perspCorrectMatQ[1][0] * redParameters[1] +
		perspCorrectMatQ[2][0] * redParameters[2];
	float AGreen =
		perspCorrectMatQ[0][0] * greenParameters[0] +
		perspCorrectMatQ[1][0] * greenParameters[1] +
		perspCorrectMatQ[2][0] * greenParameters[2];
	float ABlue =
		perspCorrectMatQ[0][0] * blueParameters[0] +
		perspCorrectMatQ[1][0] * blueParameters[1] +
		perspCorrectMatQ[2][0] * blueParameters[2];

	float BRed =
		perspCorrectMatQ[0][1] * redParameters[0] +
		perspCorrectMatQ[1][1] * redParameters[1] +
		perspCorrectMatQ[2][1] * redParameters[2];
	float BGreen =
		perspCorrectMatQ[0][1] * greenParameters[0] +
		perspCorrectMatQ[1][1] * greenParameters[1] +
		perspCorrectMatQ[2][1] * greenParameters[2];
	float BBlue =
		perspCorrectMatQ[0][1] * blueParameters[0] +
		perspCorrectMatQ[1][1] * blueParameters[1] +
		perspCorrectMatQ[2][1] * blueParameters[2];

	float CRed =
		perspCorrectMatQ[0][2] * redParameters[0] +
		perspCorrectMatQ[1][2] * redParameters[1] +
		perspCorrectMatQ[2][2] * redParameters[2];
	float CGreen =
		perspCorrectMatQ[0][2] * greenParameters[0] +
		perspCorrectMatQ[1][2] * greenParameters[1] +
		perspCorrectMatQ[2][2] * greenParameters[2];
	float CBlue =
		perspCorrectMatQ[0][2] * blueParameters[0] +
		perspCorrectMatQ[1][2] * blueParameters[1] +
		perspCorrectMatQ[2][2] * blueParameters[2];

	float D = perspCorrectMatQ[0][0] + perspCorrectMatQ[1][0] + perspCorrectMatQ[2][0];
	float E = perspCorrectMatQ[0][1] + perspCorrectMatQ[1][1] + perspCorrectMatQ[2][1];
	float F = perspCorrectMatQ[0][2] + perspCorrectMatQ[1][2] + perspCorrectMatQ[2][2];

	// final raster parameter interpolated result stored here
	float interpolatedS, interpolatedT;
	float interpolatedZBufferDepth;
	unsigned int texelColor;

	// abc coefficients for the raster parameter interpolation
	// in screen space
	V3 abcDepth = baryMatrixInverse * oneOverWParameters;

	float a[3], b[3], c[3]; // a,b,c for the three triangle edge expressions

	// establish the three edge equations 
	// edge that goes through vertices 0 and 1
	a[0] = yCoords[1] - yCoords[0];
	b[0] = -xCoords[1] + xCoords[0];
	c[0] = -xCoords[0] * yCoords[1] + yCoords[0] * xCoords[1];
	// edge that goes through vertices 1 and 2
	a[1] = yCoords[2] - yCoords[1];
	b[1] = -xCoords[2] + xCoords[1];
	c[1] = -xCoords[1] * yCoords[2] + yCoords[1] * xCoords[2];
	// edge that goes through vertices 2 and 0
	a[2] = yCoords[0] - yCoords[2];
	b[2] = -xCoords[0] + xCoords[2];
	c[2] = -xCoords[2] * yCoords[0] + yCoords[2] * xCoords[0];

	// temporary used to establish correct sidedness
	float sidedness;
	sidedness = a[0] * xCoords[2] + b[0] * yCoords[2] + c[0];
	// I need to investigate if this special case has to do with the
	// order in which the vertices are specified (i.e., cw vs ccw)
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[0] = -a[0];
		b[0] = -b[0];
		c[0] = -c[0];
	}
	sidedness = a[1] * xCoords[0] + b[1] * yCoords[0] + c[1];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[1] = -a[1];
		b[1] = -b[1];
		c[1] = -c[1];
	}
	sidedness = a[2] * xCoords[1] + b[2] * yCoords[1] + c[2];
	if (sidedness < 0) { // special case, normally inside half space is positive
						 // flip
		a[2] = -a[2];
		b[2] = -b[2];
		c[2] = -c[2];
	}

	// use clipped AABB
	int left = (int)(bbox[0][0] + 0.5);
	int right = (int)(bbox[1][0] - 0.5);
	int top = (int)(bbox[1][1] + 0.5);
	int bottom = (int)(bbox[0][1] - 0.5);

	int currPixX, currPixY; // current pixel considered
	float currEELS[3], currEE[3]; // edge expression values for the line starts and within line
	for (currPixY = top,
		currEELS[0] = a[0] * (left + 0.5f) + b[0] * (top + 0.5f) + c[0],
		currEELS[1] = a[1] * (left + 0.5f) + b[1] * (top + 0.5f) + c[1],
		currEELS[2] = a[2] * (left + 0.5f) + b[2] * (top + 0.5f) + c[2];
		currPixY <= bottom;
		currPixY++,
		currEELS[0] += b[0],
		currEELS[1] += b[1],
		currEELS[2] += b[2]) {
		for (currPixX = left,
			currEE[0] = currEELS[0],
			currEE[1] = currEELS[1],
			currEE[2] = currEELS[2];
			currPixX <= right;
			currPixX++,
			currEE[0] += a[0],
			currEE[1] += a[1],
			currEE[2] += a[2]) {

			if (currEE[0] < 0 || currEE[1] < 0 || currEE[2] < 0) {
				continue; // outside triangle
			}
			else { // found pixel inside of triangle; set it to interpolated color

				   // use model space linear interpolation for the s,t raster parameters
				interpolatedS =
					((As * currPixX) + (Bs * currPixY) + Cs) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedT =
					((At * currPixX) + (Bt * currPixY) + Ct) /
					((D * currPixX) + (E * currPixY) + F);

				// interpolate colors here
				V3 interpolatedColor;
				// find interpolated color by following the model space formula
				// for rater parameter linear interpolation 
				// r = ((A * u) + (B * v) + C) / ((D * u) + (E * v) + F)
				interpolatedColor[0] =
					((ARed * currPixX) + (BRed * currPixY) + CRed) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[1] =
					((AGreen * currPixX) + (BGreen * currPixY) + CGreen) /
					((D * currPixX) + (E * currPixY) + F);
				interpolatedColor[2] =
					((ABlue * currPixX) + (BBlue * currPixY) + CBlue) /
					((D * currPixX) + (E * currPixY) + F);

				// use screen space linear interpolation for the depth since we are using 1/w
				interpolatedZBufferDepth = abcDepth[0] * currPixX + abcDepth[1] * currPixY + abcDepth[2];

				// sample texture using lerped result of s,t raster parameters (in model space)
				texelColor = texture.sampleTexNearClamp(interpolatedS, interpolatedT);

				// override models vertex colors
				interpolatedColor.setFromColor(texelColor);

				// test sprite support (alpha based) works
				unsigned char alpha = ((unsigned char*)(&texelColor))[3];
				if (alpha > 0) 
				{
					float alphaModulation = (float)(alpha);
					alphaModulation /= 255.0f;
					interpolatedColor[0] = interpolatedColor[0] * alphaModulation;
					interpolatedColor[1] = interpolatedColor[1] * alphaModulation;
					interpolatedColor[2] = interpolatedColor[2] * alphaModulation;
					setIfOneOverWCloser(V3((float)currPixX, (float)currPixY, interpolatedZBufferDepth),
						interpolatedColor);
				}

			}
		}
	}
}

void FrameBuffer::draw2DSegment(const V3 &v0, const V3 &c0, const V3 &v1, const V3 &c1) {

	int stepsN;
	float duf = fabsf(v0.getX() - v1.getX());
	float dvf = fabsf(v0.getY() - v1.getY());
	if (duf > dvf) {
		// 1 because we want one pixel no matter what
		stepsN = 2 + (int)duf;
	}
	else {
		// 1 because we want one pixel no matter what
		stepsN = 2 + (int)dvf;
	}

	// lerp point along segment as well as its color
	for (int i = 0; i < stepsN; i++) {
		float frac = (float)i / (float)(stepsN - 1);
		V3 p = v0 + (v1 - v0)*frac;
		V3 c = c0 + (c1 - c0)*frac;
		
		setIfOneOverWCloser(p, c);
		//setSafe((int)p[0], (int)p[1], c.getColor()); // ignores z-buffer
	}
}

void FrameBuffer::saveAsPng(string fname) const {

	string fullDirName("pngs\\" + fname);
	vector<unsigned char> image;
	image.reserve(w * h * 4);

	unsigned char red, green, blue, alpha;
	unsigned int color;

	// copy framebuffer into a more 
	// friendly format for png library
	// Also, there is the need to flip
	// the framebuffer upside down in order 
	// to get the correct image
	for (int i = (h-1); i >= 0; i--) {
		for (int j = 0; j < w; j++) {

			color = pix[(i*w) + j];
			red = ((unsigned char*)(&color))[0];
			green = ((unsigned char*)(&color))[1];
			blue = ((unsigned char*)(&color))[2];
			alpha = ((unsigned char*)(&color))[3];
			image.push_back(red);
			image.push_back(green);
			image.push_back(blue);
			image.push_back(alpha);
		}
	}

	// encode the image as a png
	unsigned error = lodepng::encode(fullDirName.c_str(), image, w, h);

	// if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	
}

void FrameBuffer::loadFromPng(string fname) {
	
	vector<unsigned char> image; //the raw pixels
	unsigned int width, height;

	//decode
	unsigned int error = lodepng::decode(image, width, height, fname.c_str());

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	// Only load if it fits in the framebuffer
	if (width <= (unsigned int) w && height <= (unsigned int) h) {
		unsigned char red, green, blue, alpha;
		unsigned int color;

		// copy image into framebuffer 
		for (unsigned int i = 0, ii = 0; i < height; i++, ii += 4) {
			for (unsigned int j = 0, jj = 0; j < width; j++, jj += 4) {

				red = image[ii*width + jj + 0];
				green = image[ii*width + jj + 1];
				blue = image[ii*width + jj + 2];
				alpha = image[ii*width + jj + 3];			

				((unsigned char*)(&color))[0] = red;
				((unsigned char*)(&color))[1] = green;
				((unsigned char*)(&color))[2] = blue;
				((unsigned char*)(&color))[3] = alpha;

				set(j, i, color);
			}
		}
	}
	else {
		std::cout << "decoder error : Image is larger than the framebuffer, please rescale.." << std::endl;
	}

}