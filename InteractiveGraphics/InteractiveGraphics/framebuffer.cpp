#include "framebuffer.h"
#include "lodepng.h"
#include "scene.h"
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

}

FrameBuffer::~FrameBuffer()
{
	delete[] pix;
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

  int key = Fl::event_key();
  switch (key) {
    case FL_Left: {
      cerr << "pressed left arrow" << endl;
      break;
    }
    case 'a': {
      cerr << "pressed a" << endl;
      break;
    }
    default:
    cerr << "INFO: do not understand keypress" << endl;
  }
}

// clear to background color
void FrameBuffer::Set(unsigned int color) {

  for (int i = 0; i < w*h; i++) {
    pix[i] = color;
  }

}

// set pixel with coordinates u v to color provided as parameter
void FrameBuffer::SetSafe(int u, int v, unsigned int color) {

  if (u < 0 || u > w-1 || v < 0 || v > h-1)
    return;

  Set(u, v, color);

}

void FrameBuffer::Set(int u, int v, unsigned int color) {

  pix[(h-1-v)*w+u] = color;

}

// set to checkboard
void FrameBuffer::SetCheckerboard(int checkerSize, unsigned int color0, 
  unsigned int color1) {

  for (int v = 0; v < h; v++) {
    for (int u = 0; u < w; u++) {
      int cu = u / checkerSize;
      int cv = v / checkerSize;
      if (((cu + cv) % 2) == 0) {
        Set(u, v, color0);
      }
      else {
        Set(u, v, color1);
      }
    }
  }

}

// draw circle
void FrameBuffer::DrawCircle(float cuf, float cvf, float radius, 
  unsigned int color) {

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
        Set(u, v, color);
      }
    }
}

// draw axis aligned rectangle
void FrameBuffer::DrawRectangle(
	float llu, 
	float llv, 
	float width, 
	float height,		
	unsigned int color) {

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
			Set(u, v, color);
		}
	}
}

void FrameBuffer::DrawTriangle(float * xCoords, float * yCoords, unsigned int color)
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
				Set(currPixX, currPixY, color);
			}
		}
	}
}

void FrameBuffer::SaveAsPng(string fname) {

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
	unsigned error = lodepng::encode(fname.c_str(), image, w, h);

	// if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	
}

void FrameBuffer::LoadFromPng(string fname) {
	
	vector<unsigned char> image; //the raw pixels
	unsigned width, height;

	//decode
	unsigned error = lodepng::decode(image, width, height, fname.c_str());

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

				//pix[(i*width) + j] = color;

				Set(j, i, color);
			}
		}
	}
	else {
		std::cout << "decoder error : Image is larger than the framebuffer, please rescale.." << std::endl;
	}

}