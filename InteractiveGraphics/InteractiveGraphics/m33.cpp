#define _USE_MATH_DEFINES // need this for visual studio to find M_PI
#include <cmath>
#include "m33.h"

M33::M33(const V3 &row1, const V3 &row2, const V3 &row3)
{
	rows[0] = row1;
	rows[1] = row2;
	rows[2] = row3;
}

M33::M33(const M33 & matrixToCopy)
{
	rows[0] = matrixToCopy.rows[0];
	rows[1] = matrixToCopy.rows[1];
	rows[2] = matrixToCopy.rows[2];
}

M33::M33()
{
	rows[0] = V3();
	rows[1] = V3();
	rows[2] = V3();
}

const M33 & M33::operator=(const M33 & right)
{
	rows[0] = right.rows[0];
	rows[1] = right.rows[1];
	rows[2] = right.rows[2];
	return *this; // allows for m1 = m2 = m3
}

// access to rows for reading and writing
V3& M33::operator[](int i) {

  return rows[i];
}

ostream& operator<<(ostream &output, const M33 &matrix) {

	// outputs a matrix with the following format
	// [a11 a12 a13
	//  a21 a22 a23
	//  a31 a32 a33]
	output << "[";
	for (int i = 0; i < 3; i++) {
		output << matrix.rows[i].getX() << " " << matrix.rows[i].getY() << " " << matrix.rows[i].getZ() << '\n';
		if(i != 2)
			output << " ";
	}
	output << "]";
	return output; // enables cout << a << b << c;
}

istream& operator>>(istream &input, M33 &matrix) {

	// assumes a matrix is input with the following format
	// [a11, a12, a13, a21, a22, a23, a31, a32, a33]
	input.ignore(); // skip '['
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {

			input >> matrix.rows[i][j];
			if (i != 2 && j != 2)
				input.ignore(2); // skip ', '
			else
				input.ignore(1); // skip ', '
		}
	}
	return input; // enables cin >> a >> b >> c;
}

const V3 M33::operator*(const V3 &vector) const {

	// option 1 fast
	float x, y, z;
	// matrix vector mult implemented with dot products
	x = rows[0] * vector;
	y = rows[1] * vector;
	z = rows[2] * vector;
	return V3(x, y, z);

	// option 2 (more elegant)
	// V3 ret;
	// M33 &m = *this;
	// ret[0] = m[0] * vector;
	// ret[1] = m[0] * vector;
	// ret[2] = m[0] * vector;
	// return ret;
}

const M33 M33::operator*(const M33 &matrix) const {

	// matrix matrix multiplication implemented with dot products
	M33 returnMatrix;
	// first row
	returnMatrix[0][0] = rows[0] * matrix.getColumn(0);
	returnMatrix[0][1] = rows[0] * matrix.getColumn(1);
	returnMatrix[0][2] = rows[0] * matrix.getColumn(2);
	// second row
	returnMatrix[1][0] = rows[1] * matrix.getColumn(0);
	returnMatrix[1][1] = rows[1] * matrix.getColumn(1);
	returnMatrix[1][2] = rows[1] * matrix.getColumn(2);
	// third row
	returnMatrix[2][0] = rows[2] * matrix.getColumn(0);
	returnMatrix[2][1] = rows[2] * matrix.getColumn(1);
	returnMatrix[2][2] = rows[2] * matrix.getColumn(2);

	return returnMatrix;
}

V3 M33::getColumn(int j) const {

	V3 returnColumnVector;
	
	for (int i = 0; i < 3; i++) {
		// have to use the exclusive read func as opposed to using read/write
		// [] operator in order to honor the const in this func signature
		returnColumnVector[i] = rows[i].getComp(j);
	}
	return returnColumnVector;
}

void M33::setColumn(const V3 &columnVector, int j) {

	for (int i = 0; i < 3; i++) {
		// have to use the exclusive read func as opposed to using read/write
		// [] operator in order to honor the const in this func signature
		rows[i][j] = columnVector.getComp(i);
	}
}

M33 M33::getInverted(void) const {

	// This formaula gets inverse matrix of any matrix, remember
	// that the inverse matrix of a rotation matrix is its transpose
	// so no need to call this function when dealing exclusively with 
	// rot matrices
	M33 ret;
	V3 a = getColumn(0), b = getColumn(1), c = getColumn(2);
	V3 _a = b ^ c; _a = _a / (a * _a);
	V3 _b = c ^ a; _b = _b / (b * _b);
	V3 _c = a ^ b; _c = _c / (c * _c);
	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;
}

void M33::setInverted(void) {

	M33 inverted = getInverted();
	*this = inverted;
}

M33 M33::getTranspose(void) const {

	M33 returnMatrix;

	returnMatrix.rows[0] = this->getColumn(0);
	returnMatrix.rows[1] = this->getColumn(1);
	returnMatrix.rows[2] = this->getColumn(2);
	return returnMatrix;
}

void M33::setRotationAboutX(float theta) {

	float thetaRadians = (theta * (float)M_PI) / 180.0f;
	rows[0] = V3(1.0f, 0.0f, 0.0f);
	rows[1] = V3(0.0f, cos(thetaRadians), -sin(thetaRadians));
	rows[2] = V3(0.0f, sin(thetaRadians), cos(thetaRadians));
}

void M33::setRotationAboutY(float theta) {

	float thetaRadians = (theta * (float) M_PI) / 180.0f;
	rows[0] = V3(cos(thetaRadians), 0.0f, sin(thetaRadians));
	rows[1] = V3(0.0f, 1.0f, 0.0f);
	rows[2] = V3(-sin(thetaRadians), 0.0f, cos(thetaRadians));
}

void M33::setRotationAboutZ(float theta) {

	float thetaRadians = (theta * (float)M_PI) / 180.0f;
	rows[0] = V3(cos(thetaRadians), -sin(thetaRadians), 0.0f);
	rows[1] = V3(sin(thetaRadians), cos(thetaRadians), 0.0f);
	rows[2] = V3(0.0f, 0.0f, 1.0f);
}