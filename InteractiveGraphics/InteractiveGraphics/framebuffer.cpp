#include "framebuffer.h"
#include "scene.h"
#include "lodepng.h"
#include "scene.h"
#include "m33.h"
#include <iostream>
#include <math.h>
#include <cfloat>
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
void FrameBuffer::setIfCloser(const V3 & p, const V3 & c)
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
			setIfCloser(V3(uf,vf, p.getZ()), color);
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

void FrameBuffer::draw2DFlatTriangle(const float * xCoords, const float * yCoords, unsigned int color)
{
	float a[3], b[3], c[3]; // a,b,c for the three edge expressions
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
		return;
	if (bbox[0][0] < 0)
		bbox[0][0] = 0;
	if (bbox[1][0] > w - 1)
		bbox[1][0] = (float) w - 1;
	if (bbox[1][1] < 0)
		bbox[1][1] = 0;
	if (bbox[0][1] > h - 1)
		bbox[0][1] = (float) h - 1;

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
			else {
				// found pixel inside of triangle; set it to right color
				set(currPixX, currPixY, color);
			}
		}
	}
}

void FrameBuffer::draw2DFlatBarycentricTriangle(
	const V3 &v1, const V3 &c1,
	const V3 &v2, const V3 &c2,
	const V3 &v3, const V3 &c3)
{
	float xCoords[3], yCoords[3]; // this format is more triangle edge equation friendly 
	xCoords[0] = v1.getX();
	xCoords[1] = v2.getX();
	xCoords[2] = v3.getX();
	yCoords[0] = v1.getY();
	yCoords[1] = v2.getY();
	yCoords[2] = v3.getY();

	// build rasterization parameters to be lerped in screen space
	V3 redParameters(c1.getX(), c2.getX(), c3.getX());
	V3 greenParameters(c1.getY(), c2.getY(), c3.getY());
	V3 blueParameters(c1.getZ(), c2.getZ(), c3.getZ());
	V3 oneOverWParameters(v1.getZ(), v2.getZ(), v3.getZ());

	// build barycentric interpolation matrix
	M33 baryMatrixInverse(
		V3(xCoords[0], yCoords[0], 1),
		V3(xCoords[1], yCoords[1], 1),
		V3(xCoords[2], yCoords[2], 1));
	baryMatrixInverse.setInverted();

	// abc coefficients for the raster parameter interpolation
	V3 abcRed = baryMatrixInverse * redParameters;
	V3 abcGreen = baryMatrixInverse * greenParameters;
	V3 abcBlue = baryMatrixInverse * blueParameters;
	V3 abcDepth = baryMatrixInverse * oneOverWParameters;

	// final raster parameter interpolated result stored here
	V3 interpolatedColor;
	float interpolatedZBufferDepth = 0.0f;

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
		return;
	if (bbox[0][0] < 0)
		bbox[0][0] = 0;
	if (bbox[1][0] > w - 1)
		bbox[1][0] = (float)w - 1;
	if (bbox[1][1] < 0)
		bbox[1][1] = 0;
	if (bbox[0][1] > h - 1)
		bbox[0][1] = (float)h - 1;

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
				
				// find interpolated color by plugging abc coefficients and u,vs into
				// barycentric color interpolation equation
				interpolatedColor[0] = abcRed[0] * currPixX + abcRed[1] * currPixY + abcRed[2];
				interpolatedColor[1] = abcGreen[0] * currPixX + abcGreen[1] * currPixY + abcGreen[2];
				interpolatedColor[2] = abcBlue[0] * currPixX + abcBlue[1] * currPixY + abcBlue[2];

				interpolatedZBufferDepth = abcDepth[0] * currPixX + abcDepth[1] * currPixY + abcDepth[2];

				//set(currPixX, currPixY, interpolatedColor.getColor()); // ignores depth test
				setIfCloser(V3((float)currPixX, (float)currPixY, interpolatedZBufferDepth), interpolatedColor);
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
		
		setIfCloser(p, c);
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