#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include <string>
using std::string;
#include "v3.h"

// framebuffer + window class

class FrameBuffer : public Fl_Gl_Window {
private:
	unsigned int *pix; // SW color buffer
	float *zb; // zbuffer for visibility
	int w, h; // image resolution
public:
	FrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h); // constructor, top left coords and resolution
	~FrameBuffer();

	// function that is always called back by system and never called directly by programmer
	// programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
	// system call draw
	void draw();

	// simple getters
	int getWidth(void) const { return w; }
	int getHeight(void) const { return h; }

	// function called back when event occurs (mouse, keyboard, etc)
	int handle(int event);
	void KeyboardHandle();

	// clear to background color
	void set(unsigned int color);
	// clear z buffer to far distance, corresponding to background
	void clearZB(float farz);
	// set one pixel function, check for frame boundaries
	void setSafe(int u, int c, unsigned int color);
	// set one pixel function
	void set(int u, int c, unsigned int color);
	// set to checkboard
	void setCheckerboard(int checkerSize, unsigned int color0,
		unsigned int color1);
	// sets pixel to color c if wins z test
	void setIfCloser(const V3 &p, const V3 &c);

	// draw 2D circle with no depth test
	void draw2DCircle(float cuf, float cvf, float radius, unsigned int color);
	// draw 2D circle only where its closer
	void draw2DCircleIfCloser(const V3 &p, float radius, const V3 &color);
	// draw axis aligned rectangle with no depth test
	void draw2DRectangle(float llu, float llv, float width, float height, unsigned int color);
	// draw single color 2D triangle with no depth test
	// Quick note on pointers vs references: int &x =  y is the same as const int * x = &y
	void draw2DFlatTriangle(const float *uCoords, const float *vCoords, unsigned int color);
	// draw 2D triangle using barycentric interpolation of colors (screen space interpolation)
	// and depth test
	void draw2DFlatBarycentricTriangle(
		const V3 &v1, const V3 &c1,
		const V3 &v2, const V3 &c2,
		const V3 &v3, const V3 &c3);
	// draw 2D triangle using perspectively correct interpolation of colors and depth test
	void draw2DFlatPerspCorrectTriangle(
		const V3 &v1, const V3 &c1,
		const V3 &v2, const V3 &c2,
		const V3 &v3, const V3 &c3,
		M33 perspCorrectMatQ);

	// draw 2D segment specified by 2 points, each with own color
	void draw2DSegment(const V3 &v0, const V3 &c0, const V3 &v1, const V3 &c1);

	// save as png image
	void saveAsPng(string fname) const;
	// load from png image
	void loadFromPng(string fname);
};


