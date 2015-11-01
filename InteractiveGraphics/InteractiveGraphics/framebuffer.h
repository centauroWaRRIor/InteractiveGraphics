#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include <string>
using std::string;
#include "v3.h"
#include "m33.h"
#include "texture.h"
#include "light.h"
#include "lightprojector.h"

class CubeMap; // need forward declaration here

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
	unsigned int getPixAt(unsigned int index) const;
	float getZbAt(unsigned int index) const;

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
	void setIfOneOverWCloser(const V3 &p, const V3 &c);
	// sets pixel to color c if wins z test
	void setIfWCloser(const V3 &p, const V3 &c);
	// query if given 2D point (u,v,1/w) is closer that what is
	// is stored in the zb
	bool isDepthTestPass(const V3 &p, float epsilon);

	// draw 2D circle with no depth test
	void draw2DCircle(float cuf, float cvf, float radius, unsigned int color);
	// draw 2D circle only where its closer
	void draw2DCircleIfCloser(const V3 &p, float radius, const V3 &color);
	// draw axis aligned rectangle with no depth test
	void draw2DRectangle(float llu, float llv, float width, float height, unsigned int color);
	// draw single color 2D triangle with no depth test
	// Quick note on pointers vs references: int &x =  y is the same as const int * x = &y
	void draw2DFlatTriangle(
		V3 *const pvs,
		unsigned int color);
	// draw 2D triangle using barycentric interpolation of colors (screen space interpolation)
	// and depth test
	void draw2DFlatTriangleScreenSpace(
		V3 *const pvs,
		V3 *const cols);
	// draw 2D triangle using perspectively correct interpolation of colors and depth test
	void draw2DFlatTriangleModelSpace(
		V3 *const pvs,
		V3 *const cols,
		M33 perspCorrectMatQ);
	// draw 2D triangle using model space linear interpolation of s,t and 
	// screen space linar interpolation of depth test. Reason being that 1/w
	// gives us greater precision for near depth test as opposed to w which 
	// givues us better precision for far depth test which we don't care as much
	void draw2DTexturedTriangle(
		V3 *const pvs,
		V3 *const cols,
		const V3 &sCoords, 
		const V3 &tCoords,
		M33 perspCorrectMatQ,
		const Texture &texture);
	// draw sprite using model space linear interpolation of s,t and 
	// screen space linar interpolation of depth test.
	void draw2DSprite(
		V3 *const pvs,
		V3 *const cols,
		const V3 &sCoords,
		const V3 &tCoords,
		M33 perspCorrectMatQ,
		const Texture &texture);
	// draws 2D textured triangle with lighting and depth test
	// colors and texture uvs are interpolated in model space
	// while 1/w is interpolated in screen coordinates.
	// texturing is optional off by default unless a texture object
	// is passed. Simmilarly support for shadow mapping can be requested
	// optionally thorugh interface
	void draw2DLitTriangle(
		V3 *const vs,
		V3 *const pvs,
		V3 *const cols,
		const V3 *const normals,
		const Light &light,
		M33 perspCorrectMatQ,
		const V3 &sCoords,
		const V3 &tCoords,
		const Texture *const texture,
		bool isShadowMapOn,
		const PPC &cam,
		bool isLightProjOn,
		const LightProjector *const lightProj);
	// draw single color 2D triangle with depth test. Used for 
	// building the shadow maps
	void draw2DFlatTriangleWithDepth(
		V3 *const pvs,
		unsigned int color);
	// draws 2D textured triangle in stealth mode so that it gets cammouflaged 
	// with its environment (by using projective texturing)
	void draw2DStealthTriangle(
		V3 *const vs,
		V3 *const pvs,
		V3 *const cols,
		const V3 *const normals,
		const Light &light,
		M33 perspCorrectMatQ,
		bool isLightOn,
		bool isTexturedOn,
		const V3 &sCoords,
		const V3 &tCoords,
		const Texture *const texture,
		const PPC &cam,
		const LightProjector & lightProj);
	// draws a 2D triangle with metallic appereance that is reflective in nature
	// by using an environment map.
	void draw2DReflectiveTriangle(
		CubeMap &cubeMap,
		const PPC &cam,
		V3 *const vs,
		V3 *const pvs,
		V3 *const cols,
		const V3 *const normals,
		M33 perspCorrectMatQ,
		const V3 &sCoords,
		const V3 &tCoords,
		const Texture *const texture);
	// draws a 2D triangle with glassy appereance that is refractive in nature
	// by using an environment map.
	void draw2DRefractiveTriangle(
		float nl, float nt, // nl = refractive index for medium, nt = refractive index of TMesh material
		CubeMap &cubeMap,
		const PPC &cam,
		V3 *const vs,
		V3 *const pvs,
		V3 *const cols,
		const V3 *const normals,
		M33 perspCorrectMatQ,
		const V3 &sCoords,
		const V3 &tCoords,
		const Texture *const texture);

	// draw 2D segment specified by 2 points, each with own color
	void draw2DSegment(const V3 &v0, const V3 &c0, const V3 &v1, const V3 &c1);

	// draws distant geometry using an environment map
	void drawEnvironmentMap(CubeMap &cubeMap, const PPC &cam);

	// save as png image
	void saveAsPng(string fname) const;
	// load from png image
	void loadFromPng(string fname);
	// load from texture object
	void loadFromTexture(const Texture &texObj);
};


