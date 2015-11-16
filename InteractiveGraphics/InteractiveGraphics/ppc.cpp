#define _USE_MATH_DEFINES // need this for visual studio to find M_PI
#include <cmath>
#include <iostream>
using std::cerr;
using std::endl;
using std::ios;
#include <fstream>
using std::ofstream;
using std::ifstream;
#include <cstdlib>
using std::exit;
#include <GL\glut.h> // needed for GLU.h inclusion
#include <GL\GLU.h> // needed for glLookAt definition
#include "ppc.h"

void PPC::buildProjM(void)
{
	// three equations, three unknowns (look at slide 8 in PHC.pdf)
	projM.setColumn(a, 0);
	projM.setColumn(b, 1);
	projM.setColumn(c, 2);
	projM.setInverted();
}

PPC::PPC(float _hfovDeg, int _w, int _h) :
	w(_w),
	h(_h)
{
	a = V3(1.0f, 0.0f, 0.0f); // pixel width vector is in the +x direction by construction
	b = V3(0.0f, -1.0f, 0.0f); // pixel height vector is in the -y direction by construction
	float hfovRadians = (_hfovDeg * (float)M_PI) / 180.0f;
	C = V3(0.0f, 0.0f, 0.0f); // eye is set at the origin by construction
	// little c is is the vector from the eye to the top left corner of the image
	c = V3((float)-w/2.0f, (float)h/2.0f, -(float)w/(2.0f*tan(hfovRadians/2.0f)));
	buildProjM();
}

PPC::PPC(string cameraFilename)
{
	loadCameraFromFile(cameraFilename);
}

V3 PPC::getViewDir(void) const
{
	// view direction is obtained by taking cross product of vectors a and b
	V3 returnVector(a ^ b);
	returnVector.normalize();
	return returnVector;
}

float PPC::getFocalLength(void) const
{
	// this is the distance from center of projection to eye position and its
	// easily calculated by projecting vector c onto view direction
	V3 viewDir = getViewDir();
	return (viewDir*c);
}


V3 PPC::getRayForPixel(int u, int v) const
{
	// ray(u,v) = a * (u + 0.5) + b * ( v + 0.5) + c
	V3 aScaled = a * (u + 0.5f);
	V3 bScaled = b * (v + 0.5f);
	return (aScaled + bScaled + c);
}

V3 PPC::getRayForImagePoint(float uf, float vf) const
{
	// ray(u,v) = a*uf + b*vf + c
	V3 aScaled = a * uf;
	V3 bScaled = b * vf;
	return (aScaled + bScaled + c);
}

V3 PPC::getPixelCenter(int u, int v) const
{
	return (C + getRayForPixel(u,v));
}

float PPC::getHfovDeg(void) const
{
	// Can't store hfov given at construction time and just return here
	// because focal length may change if the user applied zoom operations.
	// Therefore it needs to be recomputed.
	// Following formula assumes C projects at w/2. a.length is in case we
	// are not dealing with square pixels
	float focalLength = getFocalLength();
	float hfovRadians = 2 * atan(w / 2 * a.length() / focalLength);
	// return degrees
	return (hfovRadians * 180.0f) / (float)M_PI;
}

V3 PPC::getPrincipalPoint(void) const
{
	// this is a general formula that handles the case
	// when the camera does not have a square pixel of unit length
	// (a and b are not magnitude 1)

	// create copies here to honor const
	V3 aNormalized = a;
	V3 bNormalized = b;
	// again in case the pixel is not square unit length
	aNormalized.normalize();
	bNormalized.normalize();
	float PPu = -1.0f * (c * aNormalized) / a.length();
	float PPv = -1.0f * (c * bNormalized) / b.length();
	return V3(PPu, PPv, 0.0f);
}

void PPC::translate(const V3 & transVector)
{
	C = C + transVector;
}

void PPC::moveRight(float step)
{
	// assumes a is unit length
	C = (C + a*step);
}

void PPC::moveUp(float step)
{
	// assumes b is unit length
	C = (C - b*step);
}

void PPC::moveForward(float step)
{
	C = (C + (getViewDir() * step));
}

void PPC::pan(float angleDeg)
{
	// b will lose precision after other rotation operations so renormalize
	b.normalize();
	a.rotateThisVectorAboutDirection(-1.0f * b, angleDeg);
	b.rotateThisVectorAboutDirection(-1.0f * b, angleDeg);
	c.rotateThisVectorAboutDirection(-1.0f * b, angleDeg);
	// update projection matrix
	buildProjM();
}

void PPC::tilt(float angleDeg)
{
	// a will lose precision after other rotation operations so renormalize
	a.normalize();
	a.rotateThisVectorAboutDirection(a, angleDeg);
	b.rotateThisVectorAboutDirection(a, angleDeg);
	c.rotateThisVectorAboutDirection(a, angleDeg);
	// update projection matrix
	buildProjM();
}

void PPC::roll(float angleDeg)
{
	V3 viewDir = getViewDir();
	a.rotateThisVectorAboutDirection(viewDir, angleDeg);
	b.rotateThisVectorAboutDirection(viewDir, angleDeg);
	c.rotateThisVectorAboutDirection(viewDir, angleDeg);
	// update projection matrix
	buildProjM();
}

void PPC::zoom(float zoomFactor)
{
	// calculate new focal length
	float newFocalLength = getFocalLength() * zoomFactor;
	// calculate new c
	V3 principalPoint = getPrincipalPoint();
	V3 viewDirection = getViewDir();
	float PPu = principalPoint.getX();
	float PPv = principalPoint.getY();
	c = -1.0f * (PPu * a) - (PPv * b) +
		viewDirection*newFocalLength;
	// update projection matrix
	buildProjM();
}

void PPC::positionRelativeToPoint(
	const V3 & P, 
	const V3 & vd, 
	const V3 & up, 
	float distance)
{
	// assumes: up and vd are normalized
	// quit early if assumptions are not met
	if (!((fabs(vd.length() - 1.0f)) < epsilonNormalizedError) ||
		!((fabs(up.length() - 1.0f)) < epsilonNormalizedError)) {
		cerr << "ERROR: Up or vd vectors are not normal vectors. Camera positioning aborted..." << endl;
		return;
	}

	V3 newa, newb, newc, newC;
	// compute new C, a, and b
	newC = P - (vd * distance);
	newa = (vd ^ up).getNormalized() * a.length();
	newb = (vd ^ newa).getNormalized() * b.length();

	V3 principalPoint = getPrincipalPoint();
	float PPu = principalPoint.getX();
	float PPv = principalPoint.getY();
	// compute new c
	newc = vd*getFocalLength() - (PPu * newa) - (PPv * newb);

	// commit new values
	C = newC;
	a = newa;
	b = newb;
	c = newc;

	// update projection matrix
	buildProjM();
}

void PPC::positionAndOrient(V3 newC, V3 lookAtPoint, V3 vInVPlane) {

	// assumes square pixels, and that the principal point (PPu,PPv)
	// projects right at the center of the screen.

	// we need to compute the camera elements for the new position and orientation
	V3 newa, newb, newc; // newC is given

	V3 newVD = (lookAtPoint - newC);
	newVD.normalize();
	newa = (newVD ^ vInVPlane).getNormalized() * a.length();
	newb = (newVD ^ newa).getNormalized()*b.length();
	newc = newVD*getFocalLength() - newa*(float)w / 2.0f - newb*(float)h / 2.0f;

	// commit new values
	C = newC;
	a = newa;
	b = newb;
	c = newc;

	// update projection matrix
	buildProjM();
}

void PPC::setByInterpolation(PPC  &ppc0, PPC &ppc1, int i, int n)
{
	// assumption is that ppc0 and ppc1 have the same internal parameters
	float t = (float)i / (float)(n - 1);
	// Ci
	C = ppc0.C + (ppc1.C - ppc0.C) * t;
	// vdi
	V3 vd0 = ppc0.getViewDir();
	V3 vd1 = ppc1.getViewDir();
	V3 vdi = vd0 + (vd1 - vd0) * t;
	// ai
	a = ppc0.a + (ppc1.a - ppc0.a) * t;
	// in order to calculate b and c we use the camera positioning 
	// formulation. These formulas require us to know internal
	// camera parameters such as PPu, PPv, f, a.length and b.length.
	// But since we are assuming both endpoint cameras have the same
	// internals we can use camera zero as reference
	float PPu = ppc0.getPrincipalPoint().getX();
	float PPv = ppc0.getPrincipalPoint().getY();
	float f = ppc0.getFocalLength();
    // we could assume this is always one in our case but are calculated
	// here anyways just to follow the slides which apply for more general cameras
	float bLength = ppc0.b.length();
	// bi
	V3 orthoNormalVector = vdi ^ a;
	orthoNormalVector.normalize();
	b = orthoNormalVector * bLength;
	// ci
	c = -1.0f * (PPu * a) - (PPv * b) +
		vdi*f;
	// update projection matrix
	buildProjM();
}

void PPC::visualizeCamera(const PPC & visCam, SWFrameBuffer & fb, float visF)
{
	float scf = visF / getFocalLength();
	V3 c0(0.0f, 0.2f, 1.0f);
	V3 c1(0.0f, 0.0f, 0.0f);
	V3 v0, v1, projv0, projv1;
	bool isInViewFrustrum;
	
	// Draw camera projection plane
	v0 = C + c*scf;
	v1 = C + (c + a*(float)w)*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c1, projv1, c1);

	v0 = C + (c + a*(float)w)*scf;
	v1 = C + (c + a*(float)w + b*(float)h)*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c1, projv1, c1);

	v0 = C + (c + a*(float)w + b*(float)h)*scf;
	v1 = C + (c + b*(float)h)*scf, c1;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c1, projv1, c1);

	v0 = C + (c + b*(float)h)*scf;
	v1 = C + c*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c1, projv1, c1);

	// draw lines connecting to eye
	v0 = C;
	v1 = C + c*scf, c1;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c0, projv1, c1);

	v0 = C;
	v1 = C + (c + a*(float)w)*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c0, projv1, c1);

	v0 = C;
	v1 = C + (c + a*(float)w + b*(float)h)*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c0, projv1, c1);

	v0 = C;
	v1 = C + (c + b*(float)h)*scf;
	isInViewFrustrum = visCam.project(v0, projv0);
	isInViewFrustrum &= visCam.project(v1, projv1);
	fb.draw2DSegment(projv0, c0, projv1, c1);
}

void PPC::setGLIntrinsics(float nearValue, float farValue) const
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float f = getFocalLength();
	float scalef = nearValue / f;
	float wf = getLowerCaseA().length() * (float) w;
	float hf = getLowerCaseB().length() * (float)h;
	glFrustum(
		-wf / 2.0f * scalef, wf / 2.0f * scalef, // left - right
		-hf / 2.0f * scalef, hf / 2.0f * scalef,  // down - top
		nearValue, farValue);
	glMatrixMode(GL_MODELVIEW);
}

#if 0
//This is how glLookAt could be implemented (tested and works fine)
static void gluLookAt(
	float eyex, float eyey, float eyez,
	float centerx, float centery, float centerz,
	float upx, float upy, float upz)
{
	V3 center, eye;
	V3 forward, side, up;
	GLfloat m[4][4];

	center[0] = centerx;
	center[1] = centery;
	center[2] = centerz;

	eye[0] = eyex;
	eye[1] = eyey;
	eye[2] = eyez;

	forward = center - eye;

	up[0] = upx;
	up[1] = upy;
	up[2] = upz;

	forward.normalize();

	// Side = forward x up
	side = forward ^ up;
	side.normalize();

	// Recompute up as: up = side x forward
	up = side ^ forward;

	// Set up transformation matrix
	m[0][0] = side[0];
	m[1][0] = side[1];
	m[2][0] = side[2];
	m[3][0] = 0.0f;

	m[0][1] = up[0];
	m[1][1] = up[1];
	m[2][1] = up[2];
	m[3][1] = 0.0f;

	m[0][2] = -forward[0];
	m[1][2] = -forward[1];
	m[2][2] = -forward[2];
	m[3][2] = 0.0f;

	m[0][3] = 0.0f;
	m[1][3] = 0.0f;
	m[2][3] = 0.0f;
	m[3][3] = 1.0f;

	glMultMatrixf(&m[0][0]);
	glTranslatef(-eyex, -eyey, -eyez);
}
#endif

void PPC::setGLExtrinsics(void) const
{
	const float lapFactor = 100.0f;
	V3 lap = C + getViewDir() * lapFactor;
	V3 up =  getLowerCaseB().getNormalized() * -1.0f;
	// assuming current matrix mode is GL_MODELVIEW
	glLoadIdentity();
	gluLookAt(
		C.getX(), C.getY(), C.getZ(), 
		lap[0], lap[1], lap[2], 
		up[0], up[1], up[2]);
}

// projects given point, returns false if point behind head
bool PPC::project(const V3 &P, V3& projP) const {
	
	// project point (assunes projM is up to date)
	V3 q = projM * (P - C);

	// no projection since point is behind camera or exactly at C
	if (q[2] <= 0.0f)
		return false;

	// remember that q[2] = w
	projP[0] = q[0] / q[2]; // u coordinate of projected point 
	projP[1] = q[1] / q[2]; // v coordinate of projected point
	projP[2] = 1.0f / q[2]; // 1/w of projected point, to be used later for visibility

	return true;
}

V3 PPC::unproject(const V3 & projP) const
{
	// From projection of point formula we know
	// P = C + (au + bv + c) * w
	// but since our project function above returns 1/w
	// P = C + (au + bv + c) * (1/w)
	V3 ret = C + (a*projP.getX() + b*projP.getY() + c) / projP.getZ();

	return ret;
}

void PPC::loadCameraFromFile(string fName)
{
	// ifstream constructor opens the file
	ifstream inFile(fName, ios::in);

	// exit program if unable to create file
	if (!inFile) { // overloaded ! operator
		cerr << "Camera saved file could not be loaded" << endl;
		exit(1);
	}

	// read camera contents to file sequentially
	inFile >> a;
	inFile >> b;
	inFile >> c;
	inFile >> C;
	inFile >> w;
	inFile >> h;

	buildProjM();

	return; // ifstram destructor closes file
}

void PPC::saveCameraToFile(string fName) const
{
	string fullDirName("camera_saves\\" + fName);
	// ofstream constructor opens a file
	ofstream outFile(fullDirName, ios::out);

	// exit program if unable to create file
	if (!outFile) { // overloaded ! operator
		cerr << "Camera save file could not be written" << endl;
		exit(1);
	}

	// write camera contents to file sequentially
	string instructions("From top row to bottom: a, b, c, C, w, h");
	
	outFile << a << endl;
	outFile << b << endl;
	outFile << c << endl;
	outFile << C << endl;
	outFile << w << endl;
	outFile << h << endl;
	// this needs to be at the end to avoid having to
	// tokenize all the words one by one.
	outFile << instructions << endl; 

	return; // ofstram destructor closes file
}
