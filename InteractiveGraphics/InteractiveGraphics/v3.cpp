#include <cmath>
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
      V3(xyz[1] * right.xyz[2] - right.xyz[1] * xyz[2],  // x
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
	if (scalar != 0)
		return V3(xyz[0] * scalar,
			xyz[1] * scalar,
			xyz[2] * scalar);
	else
		return V3();
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

	// TBD: May need to normalize axisDirection here or at least enforce with
	// an assert

	// step 1 create new coordinate system with axisOrigin and axisDirection
	// as one of its axis

	// decide whether to pick x-axis or y-axis to start deriving new basis
	// vectors for new coordinate system
	V3 xAxis(1.0f, 0.0f, 0.0f);
	V3 yAxis(0.0f, 1.0f, 0.0f);

	float dotProductXAxis = xAxis * axisDirection;
	float dotProductYAxis = yAxis * axisDirection;

	// choose the axis that makes the larger angle with axisDirection
	// assumes axisDirection is also a unit vector at this point
	V3 chosenAxis = dotProductXAxis < dotProductYAxis ? xAxis : yAxis;

	// derive a vector that is perpendicular to axisDirection
	V3 basisVector2 = chosenAxis ^ axisDirection;
	// derive a third vector that is perpendicular to axisDirection
	// and basisVector2
	V3 basisVector3 = axisDirection ^ basisVector2;

	// normalize in order to have an orthonormal basis set
	basisVector2.normalize();
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
	M33 tempMat = R.getTranspose(); // TODO: Delete this debug line
	P = (R.getTranspose() * tempPoint) + axisOrigin;
}

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
	input.ignore(); // skip ')'
	return input; // enables cin >> a >> b >> c;
}
