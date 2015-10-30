#include "framebuffer.h"
#include "lodepng.h"
#include "scene.h"
#include "scene.h"
#include "ppc.h"
#include "cubemap.h"
#include <iostream>
#include <math.h>
#include <cfloat> // using FLT_MAX
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


unsigned int FrameBuffer::getPixAt(unsigned int index) const
{
	if (pix)
		return pix[index];
	else
		return 0;
}

float FrameBuffer::getZbAt(unsigned int index) const
{
	if (zb)
		return zb[index];
	else
		return 0.0f;
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

bool FrameBuffer::isDepthTestPass(const V3 & p, float epsilon)
{
	if ((p.getX() < 0.0f) || (p.getX() >= w) ||
		(p.getY() < 0.0f) || (p.getY() >= h))
		return false;

	int u = (int)p.getX();
	int v = (int)p.getY();

	// remember that the z component of the projected point is 1/z or 1/w
	float zBufferValue = zb[(h - 1 - v)*w + u];
	// compute epsilon difference to make sure we are not talking about the
	// same 3D point. In other words, prevent a 3D point from occluding itself
	// in shadow by looking at itself in the shadowmap 'mirror' sort of speak.
	float difference = abs(zBufferValue - p.getZ());
	if (difference < epsilon) {
		return true;
	}
	else if (zBufferValue >= p.getZ())
		return false;
	else
		return true;
}

// draw circle
void FrameBuffer::draw2DCircle(float cuf, float cvf, float radius, 
  unsigned int color)
{
	// compute screen axes-aligned bounding box for triangle 
	// clipping against framebuffer
	AABB aabb(V3(cuf - radius + 0.5f, cvf + radius - 0.5f));
	aabb.AddPoint(V3(cuf + radius - 0.5f, cvf - radius + 0.5f));

	if (!aabb.clipWithFrame(0.0f, 0.0f, (float)w, (float)h))
		return;

	int left, right, top, bottom;
	aabb.setPixelRectangle(left, right, top, bottom);

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
	// compute screen axes-aligned bounding box for triangle 
	// clipping against framebuffer
	AABB aabb(V3(p.getX() - radius + 0.5f, p.getY() + radius - 0.5f));
	aabb.AddPoint(V3(p.getX() + radius - 0.5f, p.getY() - radius + 0.5f));

	if (!aabb.clipWithFrame(0.0f, 0.0f, (float)w, (float)h))
		return;

	int left, right, top, bottom;
	aabb.setPixelRectangle(left, right, top, bottom);

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
	// compute screen axes-aligned bounding box for triangle 
	// clipping against framebuffer
	AABB aabb(V3(llu + 0.5f, llv - 0.5f));
	aabb.AddPoint(V3(llu + width - 0.5f, llv - height + 0.5f));

	if (!aabb.clipWithFrame(0.0f, 0.0f, (float)w, (float)h))
		return;

	int left, right, top, bottom;
	aabb.setPixelRectangle(left, right, top, bottom);

	for (int v = top; v <= bottom; v++) {
		for (int u = left; u <= right; u++) {
			float uf = .5f + (float)u;
			float vf = .5f + (float)v;
			set(u, v, color);
		}
	}
}

void FrameBuffer::draw2DFlatTriangle(
	V3 *const pvs, 
	unsigned int color)
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

	// rasterize triangle
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

	// rasterize triangle
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
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DFlatTriangleModelSpace(
	V3 *const pvs,
	V3 *const cols,
	M33 Q)
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

	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(cols[0].getX(), cols[1].getX(), cols[2].getX());
	V3 greenParameters(cols[0].getY(), cols[1].getY(), cols[2].getY());
	V3 blueParameters(cols[0].getZ(), cols[1].getZ(), cols[2].getZ());
	// w is linear in model space and not linear in screen space.For 1 / w, it's the other way around.
	// Hence we want to use w here. Projected z is coming out as 1/w by construction so we want to
	// invert that z to get back to original w.
	V3 wParameters(1 / (pvs[0].getZ()), 1 / (pvs[1].getZ()), 1 / (pvs[2].getZ()));
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 depthNumABC = V3(
		Q.getColumn(0) * wParameters,
		Q.getColumn(1) * wParameters,
		Q.getColumn(2) * wParameters);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result

	// rasterize triangle
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

				// find interpolated parameter t by following the model space formula
				// for rater parameter linear interpolation 
				// t = ((A * u) + (B * v) + C) / ((D * u) + (E * v) + F)
				pixC = V3(.5f + (float)currPixU, .5f + (float)currPixV, 1.0f);
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedDepth = (depthNumABC * pixC) / denFactor;				
				setIfWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DTexturedTriangle(
	V3 *const pvs,
	V3 *const cols,
	const V3 &sCoords, 
	const V3 &tCoords,
	M33 Q,
	const Texture &texture)
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

	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(cols[0].getX(), cols[1].getX(), cols[2].getX());
	V3 greenParameters(cols[0].getY(), cols[1].getY(), cols[2].getY());
	V3 blueParameters(cols[0].getZ(), cols[1].getZ(), cols[2].getZ());
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 sNumABC = V3(
		Q.getColumn(0) * sCoords,
		Q.getColumn(1) * sCoords,
		Q.getColumn(2) * sCoords);
	V3 tNumABC = V3(
		Q.getColumn(0) * tCoords,
		Q.getColumn(1) * tCoords,
		Q.getColumn(2) * tCoords);

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result
	float interpolatedS, interpolatedT; // final raster parameter interpolated result
	unsigned int texelColor;

	// rasterize triangle
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
				// r,g,b, s, and t are interpolated in model space
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedS = (sNumABC * pixC) / denFactor;
				interpolatedT = (tNumABC * pixC) / denFactor;
				// 1/w is interpoalted in screen space
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s
				
				// sample texture using lerped result of s,t raster parameters (in model space)
				//texelColor = texture.sampleTexNearTile(interpolatedS, interpolatedT);
				texelColor = texture.sampleTexBilinearTile(interpolatedS, interpolatedT);
				// override interpolated color for now. In the future texel can be modulated by color
				interpolatedColor.setFromColor(texelColor);

				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DSprite(
	V3 *const pvs,
	V3 *const cols,
	const V3 &sCoords,
	const V3 &tCoords,
	M33 Q,
	const Texture &texture)
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

	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(cols[0].getX(), cols[1].getX(), cols[2].getX());
	V3 greenParameters(cols[0].getY(), cols[1].getY(), cols[2].getY());
	V3 blueParameters(cols[0].getZ(), cols[1].getZ(), cols[2].getZ());
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 sNumABC = V3(
		Q.getColumn(0) * sCoords,
		Q.getColumn(1) * sCoords,
		Q.getColumn(2) * sCoords);
	V3 tNumABC = V3(
		Q.getColumn(0) * tCoords,
		Q.getColumn(1) * tCoords,
		Q.getColumn(2) * tCoords);

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result
	float interpolatedS, interpolatedT; // final raster parameter interpolated result
	unsigned int texelColor;

	// rasterize triangle
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
				// r,g,b, s, and t are interpolated in model space
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedS = (sNumABC * pixC) / denFactor;
				interpolatedT = (tNumABC * pixC) / denFactor;
				// 1/w is interpoalted in screen space
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s

				 // sample texture using lerped result of s,t raster parameters (in model space)
				texelColor = texture.sampleTexNearClamp(interpolatedS, interpolatedT);

				// override interpolated color for now. In the future texel can be modulated by color
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
					setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
				}
			}
		}
	}
}

void FrameBuffer::draw2DLitTriangle(
	V3 * const vs, 
	V3 * const pvs, 
	V3 * const cols, 
	const V3 * const normals,
	const Light & light, 
	M33 Q, 
	const V3 & sCoords, 
	const V3 & tCoords, 
	const Texture * const texture,
	bool isShadowMapOn,
	const PPC &cam,
	bool isLightProjOn,
	const LightProjector *const lightProj)
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

	// compute lighting colors at 3 vertices
	V3 litCols[3];
	for (int vi = 0; vi < 3; vi++) {
		litCols[vi] = light.computeDiffuseContribution(vs[vi], normals[vi]);
	}

	if (cols != nullptr) {

		litCols[0].modulateBy(cols[0]);
		litCols[1].modulateBy(cols[1]);
		litCols[2].modulateBy(cols[2]);
	}

	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(litCols[0].getX(), litCols[1].getX(), litCols[2].getX());
	V3 greenParameters(litCols[0].getY(), litCols[1].getY(), litCols[2].getY());
	V3 blueParameters(litCols[0].getZ(), litCols[1].getZ(), litCols[2].getZ());
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 sNumABC = V3(
		Q.getColumn(0) * sCoords,
		Q.getColumn(1) * sCoords,
		Q.getColumn(2) * sCoords);
	V3 tNumABC = V3(
		Q.getColumn(0) * tCoords,
		Q.getColumn(1) * tCoords,
		Q.getColumn(2) * tCoords);

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result
	float interpolatedS, interpolatedT; // final raster parameter interpolated result
	unsigned int texelColor;

	// rasterize triangle
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
				// r,g,b, s, and t are interpolated in model space
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedS = (sNumABC * pixC) / denFactor;
				interpolatedT = (tNumABC * pixC) / denFactor;
				// 1/w is interpoalted in screen space
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s

				if (texture != nullptr) {
					// sample texture using lerped result of s,t raster parameters (in model space)
					texelColor = texture->sampleTexBilinearTile(interpolatedS, interpolatedT);
					V3 texelColorVec(texelColor);
					texelColorVec.modulateBy(interpolatedColor); // however modulate texture color against pixel lit value
					interpolatedColor = texelColorVec;
				}

				// get 3d point corresponding to this pixel
				V3 pixel3dPoint = cam.unproject(V3(pixC[0], pixC[1], interpolatedDepth));
				// do shadow mapping
				if (isShadowMapOn && light.isPointInShadow(pixel3dPoint)) {
					if(texture == nullptr) // this works without texture
						interpolatedColor = light.getMatColor() * light.getAmbientK(); 
					else // this works with texture
						interpolatedColor = interpolatedColor * light.getAmbientK();
				}

				// do projective texture mapping
				if (isLightProjOn && lightProj->getProjectedColor(pixel3dPoint, texelColor)) {
					
					V3 lightProjColor;
					lightProjColor.setFromColor(texelColor);
					unsigned char alpha = ((unsigned char*)(&texelColor))[3];
					float alphaModulation = (float)(alpha) / 255.0f;
					// make use of projective texture with alpha mask included (very useful for text)
					if (alpha > 0)
					{
						interpolatedColor += (lightProjColor * alphaModulation);
					}
				}
				// set pixel in color framebuffer as well as depth buffer if depth test passes
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DFlatTriangleWithDepth(V3 * const pvs, unsigned int color)
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

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 pColor; // final raster parameter interpolated result
	pColor.setFromColor(color);
	float interpolatedDepth; // final raster parameter interpolated result

	// rasterize triangle
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
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), pColor);
			}
		}
	}
}

void FrameBuffer::draw2DStealthTriangle(
	V3 * const vs, 
	V3 * const pvs, 
	V3 * const cols, 
	const V3 * const normals, 
	const Light & light, 
	M33 Q, 
	bool isLightOn,
	bool isTexturedOn, 
	const V3 & sCoords, 
	const V3 & tCoords, 
	const Texture * const texture, 
	const PPC & cam, 
	const LightProjector & lightProj)
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

	// lighting
	V3 litCols[3];
	if (isLightOn) {
		for (int vi = 0; vi < 3; vi++) {
			litCols[vi] = light.computeDiffuseContribution(vs[vi], normals[vi]);
		}
	}
	else {
		litCols[0] = cols[0];
		litCols[1] = cols[1];
		litCols[2] = cols[2];
	}

	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(litCols[0].getX(), litCols[1].getX(), litCols[2].getX());
	V3 greenParameters(litCols[0].getY(), litCols[1].getY(), litCols[2].getY());
	V3 blueParameters(litCols[0].getZ(), litCols[1].getZ(), litCols[2].getZ());
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 sNumABC = V3(
		Q.getColumn(0) * sCoords,
		Q.getColumn(1) * sCoords,
		Q.getColumn(2) * sCoords);
	V3 tNumABC = V3(
		Q.getColumn(0) * tCoords,
		Q.getColumn(1) * tCoords,
		Q.getColumn(2) * tCoords);

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	float interpolatedDepth; // final raster parameter interpolated result
	float interpolatedS, interpolatedT; // final raster parameter interpolated result
	unsigned int texelColor;

	// rasterize triangle
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
				// r,g,b, s, and t are interpolated in model space
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedS = (sNumABC * pixC) / denFactor;
				interpolatedT = (tNumABC * pixC) / denFactor;
				// 1/w is interpoalted in screen space
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s

				// TODO: Combine texture color with lit color instead of overriding each other
				if (isTexturedOn && texture != nullptr) {
					// sample texture using lerped result of s,t raster parameters (in model space)
					texelColor = texture->sampleTexBilinearTile(interpolatedS, interpolatedT);
					// override interpolated color for now. In the future texel can be modulated by color
					interpolatedColor.setFromColor(texelColor);
				}

				// get 3d point corresponding to this pixel
				V3 pixel3dPoint = cam.unproject(V3(pixC[0], pixC[1], interpolatedDepth));

				// TODO: Find a way to combine the colors: interpolated, texture, lit and projLight color
				// instead of overriding each other (projLight and lit color no longer override each other)
				// do projective texture mapping
				if (lightProj.getProjectedStealthColor(pixel3dPoint, texelColor)) {

					V3 lightProjColor;
					lightProjColor.setFromColor(texelColor);
					//interpolatedColor += (lightProjColor); // glass material effect (with refraction)
					interpolatedColor = (lightProjColor); // 100% stealth predator like
				}
				// set pixel in color framebuffer as well as depth buffer if depth test passes
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
			}
		}
	}
}

void FrameBuffer::draw2DReflectiveTriangle(
	CubeMap & cubeMap, 
	const PPC &cam,
	V3 * const vs, 
	V3 * const pvs, 
	V3 * const cols, 
	const V3 * const normals, 
	M33 Q, 
	const V3 & sCoords, 
	const V3 & tCoords, 
	const Texture * const texture)
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
	V3 colors[3];
	if (cols == nullptr) {
		colors[0] = V3(0.7f, 0.0f, 0.0f);
		colors[1] = V3(0.7f, 0.0f, 0.0f);
		colors[2] = V3(0.7f, 0.0f, 0.0f);
	}
	else {
		colors[0] = cols[0];
		colors[1] = cols[1];
		colors[2] = cols[2];
	}
	// set model space interpolation
	// build rasterization parameters to be lerped in screen space
	V3 redParameters(colors[0].getX(), colors[1].getX(), colors[2].getX());
	V3 greenParameters(colors[0].getY(), colors[1].getY(), colors[2].getY());
	V3 blueParameters(colors[0].getZ(), colors[1].getZ(), colors[2].getZ());
	// refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of the persp correct coefficients
	V3 denDEF = Q[0] + Q[1] + Q[2];
	V3 redNumABC = V3(
		Q.getColumn(0) * redParameters,
		Q.getColumn(1) * redParameters,
		Q.getColumn(2) * redParameters);
	V3 greenNumABC = V3(
		Q.getColumn(0) * greenParameters,
		Q.getColumn(1) * greenParameters,
		Q.getColumn(2) * greenParameters);
	V3 blueNumABC = V3(
		Q.getColumn(0) * blueParameters,
		Q.getColumn(1) * blueParameters,
		Q.getColumn(2) * blueParameters);
	V3 sNumABC = V3(
		Q.getColumn(0) * sCoords,
		Q.getColumn(1) * sCoords,
		Q.getColumn(2) * sCoords);
	V3 tNumABC = V3(
		Q.getColumn(0) * tCoords,
		Q.getColumn(1) * tCoords,
		Q.getColumn(2) * tCoords);

	// set screen space interpolation
	M33 baryMatrixInverse;
	baryMatrixInverse[0] = pvs[0];
	baryMatrixInverse[1] = pvs[1];
	baryMatrixInverse[2] = pvs[2];
	baryMatrixInverse.setColumn(V3(1.0f, 1.0f, 1.0f), 2);
	baryMatrixInverse.setInverted();
	// linear expression for screen space interpolation of 1/w
	V3 depthABC = baryMatrixInverse*V3(pvs[0][2], pvs[1][2], pvs[2][2]);

	int currPixU, currPixV; // current pixel considered
	V3 pixC; // current pixel center
	V3 topLeftPixC(left + 0.5f, top + 0.5f, 1.0f); // top left pixel center
	V3 currEELS; // edge expression values for the line start
	V3 currEE; // edge expression value within a given line 
	V3 interpolatedColor; // final raster parameter interpolated result
	V3 pixel3dPoint, E, R; // used in environment map calculation of reflected vector
	float interpolatedDepth; // final raster parameter interpolated result
	float interpolatedS, interpolatedT; // final raster parameter interpolated result
	unsigned int texelColor;

	// rasterize triangle
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
				// r,g,b, s, and t are interpolated in model space
				float denFactor = denDEF * pixC;
				interpolatedColor[0] = (redNumABC * pixC) / denFactor;
				interpolatedColor[1] = (greenNumABC * pixC) / denFactor;
				interpolatedColor[2] = (blueNumABC * pixC) / denFactor;
				interpolatedS = (sNumABC * pixC) / denFactor;
				interpolatedT = (tNumABC * pixC) / denFactor;
				// 1/w is interpoalted in screen space
				interpolatedDepth = depthABC * pixC; // 1/w at current pixel interpolated lin. in s s

				if (texture != nullptr) {
					// sample texture using lerped result of s,t raster parameters (in model space)
					texelColor = texture->sampleTexBilinearTile(interpolatedS, interpolatedT);
					V3 texelColorVec(texelColor);
					texelColorVec.modulateBy(interpolatedColor); // however modulate texture color against pixel lit value
					interpolatedColor = texelColorVec;
				}

				// get 3d point corresponding to this pixel
				pixel3dPoint = cam.unproject(V3(pixC[0], pixC[1], interpolatedDepth));

				// calculate the reflected ray R that is incident with the surface normal
				{
					// proj a onto v = ((a * v) * v) / v.length
					// if v is normalized -> proj a onto v = (a * v) v
					// apply this formula to obtain vector B in figure 10.2 of MirrorReflectionVector.pdf
					// the rest of the derivation is straightforward 
					// R = E - 2 (E * N) N where E is the incident light ray coming from the camera

					// find E

					// TODO: To calculate R we will need the interpolated normal at this point. Normalized
					// Therefore use persp correct interpolation in the same way you did for colors to obtain this
					

					// use 3d vector to find direction of ray from eye to pixel
					E = pixel3dPoint - cam.getEyePoint();
					R = E - (E * )
					E.normalize();
					// use ray's direction to look up color in environament map
				}

				// set pixel in color framebuffer as well as depth buffer if depth test passes
				setIfOneOverWCloser(V3(pixC[0], pixC[1], interpolatedDepth), interpolatedColor);
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

void FrameBuffer::drawEnvironmentMap(CubeMap & cubeMap, const PPC & cam)
{
	V3 pixC, pixC_3D, dir, color;
	// clear background to color provided by the environment map
	for (int currPixV = 0; currPixV < h; currPixV++) {
		for (int currPixU = 0; currPixU < w; currPixU++) {

			// depth value assigned to this pixel is a not care as long as its not zero
			// as zero means infinitely far away (1/w)
			pixC = V3(.5f + (float)currPixU, .5f + (float)currPixV, 1.0f);
			pixC_3D = cam.unproject(pixC);
			// use 3d vector to find direction of ray from eye to pixel
			dir = pixC_3D - cam.getEyePoint();
			dir.normalize();
			// use ray's direction to look up color in environament map
			color = cubeMap.getColor(dir);
			set(currPixU, currPixV, color.getColor());
		}
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

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA

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

void FrameBuffer::loadFromTexture(const Texture & texObj)
{
	unsigned int width, height;
	vector<unsigned char> image; //the raw pixels from texture

	image = texObj.getTexels();
	width = texObj.getTexWidth();
	height = texObj.getTexHeight();
	
	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...

	// Only load if it fits in the framebuffer
	if (width <= (unsigned int)w && height <= (unsigned int)h) {
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
		std::cout << "decoder error : texture image is larger than the framebuffer, please rescale.." << std::endl;
	}
}
