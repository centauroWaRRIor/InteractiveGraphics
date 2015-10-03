#pragma once

#include "v3.h"
#include "m33.h"
#include "framebuffer.h"
#include <string>
using std::string;

// Implements a planar pinhole camera (PPC)
class PPC {
private:
	V3 a; // pixel width vector
	V3 b; // pixel height vector
	V3 c; // vector from the eye to the top left corner of the image
	V3 C; // eye position
	int w, h; // image resolution
	// projection matrix
	M33 projM;
	// projection matrix is only built when a, b, or c change due to rotations
	void buildProjM(void);
public:
	// constructor from resolution and horizontal field of view
	PPC(float _hfovDeg, int _w, int _h);
	// constructor from saved camera file
	PPC(string cameraFilename);

	// get a
	V3 getLowerCaseA(void) const { return a; }
	// get b
	V3 getLowerCaseB(void) const { return b; }
	// get c
	V3 getLowerCaseC(void) const { return c; }
	// get eyepoint
	V3 getEyePoint(void) const { return C; }
	// get view direction
	V3 getViewDir(void) const;
	// get focal length
	float getFocalLength(void) const;
	// get ray for pixel (u,v) -- integers
	V3 getRayForPixel(int u, int v) const;
	// get ray for pixel image point (uf, vf) -- floats
	V3 getRayForImagePoint(float uf, float vf) const;
	// get pixel center -- integers
	V3 getPixelCenter(int u, int v) const;
	// get horizontal field of view in degrees
	float getHfovDeg(void) const;
	// get principal point (image coordinates of C projection onto the plane)
	// z component is always zero
	V3 getPrincipalPoint(void) const;

	// camera translations only affect C vector. a, b, and c remain intact
	void translate(const V3 &transVector); // move eye using translation vector
	void moveRight(float step); // negative left means move left
	void moveUp(float step); // negative step means move down
	void moveForward(float step); // negative step means move backward

	// camera rotations only affect a, b, and c. C vector remains intact
	void pan(float angleDeg);
	void tilt(float angleDeg);
	void roll(float angleDeg);

	// camera zooming only affects focal length and c. C, a, and b remain intact
	void zoom(float zoomFactor);

	// place camera such that it looks at point P from distance d, has view 
	// direction vd, and up is a vector in the vertical plane of the camera
	void positionRelativeToPoint(
		const V3 &P,
		const V3 &vd,
		const V3 &up,
		float distance);
	// place cemara so that it looks at point lookAtPoint from point C given
	// an up vector
	void positionAndOrient(V3 newC, V3 lookAtPoint, V3 vInVPlane);

	// projection of 3D point
	bool project(const V3 &P, V3& projP) const;

	// interpolation between two given cameras
	// sets this camera to the ith camera out of n between the 
	// interpolated result between ppc0 and ppc1
	void setByInterpolation(PPC &ppc0, PPC &ppc1, int i, int n);

	// draw camera frustum in wireframe, adapt focal length to visF
	void visualizeCamera(const PPC &visCam, FrameBuffer &fb, float visF);

	// save load from text file
	void saveCameraToFile(string fName) const;
	void loadCameraFromFile(string fName);
};