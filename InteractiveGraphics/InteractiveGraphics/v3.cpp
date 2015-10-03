#include <cmath>
#include <iostream>
using std::cerr;
using std::endl;
#include "v3.h"
#include "m33.h"

// class member function definitions

V3::V3(float x, float y, float z) {

   xyz[0] = x;
   xyz[1] = y;
   xyz[2] = z;
}

V3::V3(const V3 &vectorToCopy) {

   xyz[0] = vectorToCopy.xyz[0];
   xyz[1] = vectorToCopy.xyz[1];
   xyz[2] = vectorToCopy.xyz[2];
}

const V3& V3::operator=(const V3 &right) {

   xyz[0] = right.xyz[0];
   xyz[1] = right.xyz[1];
   xyz[2] = right.xyz[2];
   return *this;
}

float& V3::operator[](int i) {

   return xyz[i];
}

V3 V3::operator+(const V3 &right) const {

   return 
      V3(xyz[0] + right.xyz[0],  // x
         xyz[1] + right.xyz[1],  // y
         xyz[2] + right.xyz[2]); // z
}

V3 V3::operator-(const V3 &right) const {

   return 
      V3(xyz[0] - right.xyz[0],  // x
         xyz[1] - right.xyz[1],  // y
         xyz[2] - right.xyz[2]); // z
}

float V3::operator*(const V3 &right) const {

   return 
      xyz[0]*right.xyz[0] + 
      xyz[1]*right.xyz[1] + 
      xyz[2]*right.xyz[2]; // scalar
}

V3 V3::operator^(const V3 &right) const {

   return
      V3(xyz[1] * right.xyz[2] - xyz[2] * right.xyz[1],  // x
         xyz[2] * right.xyz[0] - xyz[0] * right.xyz[2],  // y
         xyz[0] * right.xyz[1] - xyz[1] * right.xyz[0]); // z
}

V3 V3::operator*(float scalar) const
{
	return V3(xyz[0] * scalar,
		xyz[1] * scalar,
		xyz[2] * scalar);
}

V3 V3::operator/(float scalar) const
{
	if (scalar != 0.0f) {
		return V3(xyz[0] / scalar,
			xyz[1] / scalar,
			xyz[2] / scalar);
	}
	else {
		cerr << "ERROR: Attempting to divide a vector by a zero scalar. Zero vector returned..." << endl;
		return V3();
	}
		
}

void V3::normalize(void)
{
	float length = this->length();
	if (length > 0) {
		xyz[0] = xyz[0] / length;
		xyz[1] = xyz[1] / length;
		xyz[2] = xyz[2] / length;
	}
}

V3 V3::getNormalized(void) const
{
	V3 result(*this);
	result.normalize();
	return result;
}

float V3::length(void) const
{
	// option A (faster)
	//return sqrt(xyz[0] * xyz[0] +
	//	xyz[1] * xyz[1] +
	//	xyz[2] * xyz[2]);

	// option B using dot product (elegant)
	return sqrt((*this) * (*this));
}

V3 V3::thisPointInNewCoordSystem(
	const V3 &origin,
	const V3 &newBasisVectorX, 
	const V3 &newBasisVectorY,
	const V3 &newBasisVectorZ) const
{
	// first translate this point to new origin,
	// then rotate old axes into new axes
	// P' =
	//		[ Xx' Xy' Xz' ]	  [ x - Ox' ]
	//		[ Yx' Yy' Yz' ] * [ y - Oy' ]
	//		[ Zx' Zy' Zz' ]	  [ z - Oz' ]

	V3 Pt(xyz[0] - origin.xyz[0],
		  xyz[1] - origin.xyz[1],
		  xyz[2] - origin.xyz[2]);

	return V3(newBasisVectorX * Pt,
			  newBasisVectorY * Pt,
			  newBasisVectorZ * Pt);
}

void V3::rotateThisPointAboutAxis(const V3 &axisOrigin, const V3 &axisDirection, float theta) {

	// axisOrigin is interpreted to be a point representing origin of axis
	// axisDirection is interpreted to be a vector representing axis direction

	// quit early if axisDirection is not unit length
	if (!((fabs(axisDirection.length()) - 1.0f) < epsilonNormalizedError)) {
		cerr << "ERROR: Attempting to rotate about a non-normalized vector. Zero vector produced..." << endl;
		*this = V3();
		return;
	}

	// step 1 create new coordinate system with axisOrigin and axisDirection
	// as one of its axis

	// decide whether to pick x-axis or y-axis to start deriving new basis
	// vectors for new coordinate system
	V3 xAxis(1.0f, 0.0f, 0.0f);
	V3 yAxis(0.0f, 1.0f, 0.0f);

	// choose the axis that makes the larger angle with axisDirection
	// assumes axisDirection is also a unit vector at this point,
	// therefore the dot product between axisDirection and the x and 
	// y axis tells the magnitude of the angle between them. Smaller
	// the dot product the greater the angle.
	V3 chosenAxis = (fabs(axisDirection.getX()) < fabs(fabs(axisDirection.getY()))) ?
			xAxis : yAxis;

	// derive a vector that is perpendicular to axisDirection
	V3 basisVector2 = chosenAxis ^ axisDirection;
	// normalize in order to have an orthonormal basis set
	basisVector2.normalize();
	// derive a third vector that is perpendicular to axisDirection
	// and basisVector2
	V3 basisVector3 = axisDirection ^ basisVector2;
	// normalize in order to have an orthonormal basis set
	basisVector3.normalize();

	// get representation of this point in new coordinate system
	// where axisDirection is the X axis
	V3 tempPoint = thisPointInNewCoordSystem(
		axisOrigin,
		axisDirection,
		basisVector2,
		basisVector3);

	// step 2 rotate point in new coordinate system rotate by theta degrees 
	// assuming axisDirection represents the X axis in new coordiante
	// system (local space)
	M33 rotMatX;
	rotMatX.setRotationAboutX(theta);
	// rotate tempPoint theta degrees about axisDirection (x-axis in local space)
	tempPoint = rotMatX * tempPoint;

	// step 3 transform back to the original system
	// according to lecture the math for this can be derived from the
	// change of coordinate system formulation and its
	// P = Inverse(R)*P' + O' = Transpose(R)*P' + O'
	// where R is the matrix to rotate old axis into new axis, which
	// is technically a rotation matrix and that's why Inverse(R)=Transpose(R)
	M33 R(axisDirection, basisVector2, basisVector3);

	// transform point back to original system
	V3 &P = *this;
	P = (R.getTranspose() * tempPoint) + axisOrigin;
}

#if 0
// Professor's implemetation is more elegant and compact
V3 V3::rotateThisPointAboutAxis(V3 Oa, V3 aDir, float theta) {
	// build aux coordinate system with axis as one of its principal axes
	V3 auxAxis;
	if (fabsf(aDir[0]) > fabsf(aDir[1])) {
		auxAxis = V3(0.0f, 1.0f, 0.0f);
	}
	else {
		auxAxis = V3(1.0f, 0.0f, 0.0f);
	}

	V3 yl = aDir;
	V3 zl = (aDir ^ auxAxis);
	zl.normalize();
	V3 xl = (yl ^ zl);
	xl.normalize();
	M33 lcs;
	lcs[0] = xl;
	lcs[1] = yl;
	lcs[2] = zl;

	// transform to aux coordinate system
	V3 &p = *this;
	V3 p1 = lcs*(p - Oa);

	// rotate about principal axis
	M33 roty;
	roty.setRotationAboutY(theta);
	V3 p2 = roty * p1;

	// transform back to old world
	V3 ret = lcs.getInverted()*p2 + Oa;
	return ret;
}
#endif

void V3::rotateThisVectorAboutDirection(const V3 &direction, float theta) {

	V3 origin(0.0f, 0.0f, 0.0f);
	this->rotateThisPointAboutAxis(origin, direction, theta);
}

unsigned int V3::getColor() const {

	unsigned int ret = 0xFF000000;

	// clamping
	const V3 &v = *this;
	for (int i = 0; i < 3; i++) {
		// clamping to [0,0f, 1.0f];
		//float cf = v[i]; // Proffessor used this
		float cf = xyz[i]; // I prefer this, allows me to use const qualifier
		unsigned char cb;
		cf = (cf < 0.0f) ? 0.0f : cf;
		cf = (cf > 1.0f) ? 1.0f : cf;
		cb = (unsigned char)(cf*255.0f + 0.5f);
		((unsigned char*)&ret)[i] = cb;
	}

	return ret;
}

float V3::getComp(int i) const
{
	if (i >= 0 && i < 3) {
		return xyz[i];
	}
	else {
		cerr << "ERROR: Attempting to get invalid vector component. Zero returned..." << endl;
		return 0.0f;
	}
}

void V3::setFromColor(unsigned int color) {

	float redf = (float)(((unsigned char*)(&color))[0]);
	redf = redf / 255.0f;
	float greenf = (float)(((unsigned char*)(&color))[1]);
	greenf = greenf / 255.0f;
	float bluef = (float)(((unsigned char*)(&color))[2]);
	bluef = bluef / 255.0f;

	xyz[0] = redf;
	xyz[1] = greenf;
	xyz[2] = bluef;
}

// friend function definitions

V3 operator*(float scalarLeft, V3 &vectorRight) {

	return (vectorRight * scalarLeft);
}

ostream& operator<<(ostream &output, const V3 &vector) {
	// outputs a vector in the following format
	// (x, y, z)
	output << "(" << vector.xyz[0] << ", " <<
		vector.xyz[1] << ", " <<
		vector.xyz[2] << ")";
	return output; // enables cout << a << b << c;
}

istream& operator>>(istream &input, V3 &vector) {
	// assumes a vector is input with the following format
	// (x, y, z)
	input.ignore(); // skip '('
	input >> vector[0];
	input.ignore(2); // skip ', '
	input >> vector[1];
	input.ignore(2); // skip ', '
	input >> vector[2];
	input.ignore(2); // skip ')'
	return input; // enables cin >> a >> b >> c;
}