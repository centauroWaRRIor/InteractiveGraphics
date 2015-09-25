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
	if (!((fabs(vd.length()) - 1.0f) < epsilonNormalizedError) ||
		!((fabs(up.length()) - 1.0f) < epsilonNormalizedError)) {
		cerr << "ERROR: Up or vd vectors are not normal vectors. Camera positioning aborted..." << endl;
		return;
	}

	// capture principal point and focal length 
	// before we modify a, b, and c
	V3 principalPoint = getPrincipalPoint();
	float focalLength = getFocalLength();

	// compute new C, a, and b
	C = P - (vd * distance);
	V3 orthoNormalVector = vd ^ up;
	orthoNormalVector.normalize();
	a = orthoNormalVector * a.length();
	orthoNormalVector = vd ^ a;
	b = orthoNormalVector * b.length();

	// compute new c
	float PPu = principalPoint.getX();
	float PPv = principalPoint.getY();
	c = -1.0f * (PPu * a) - (PPv * b) +
		vd*focalLength;

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
	// ofstream constructor opens a file
	ofstream outFile(fName, ios::out);

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
