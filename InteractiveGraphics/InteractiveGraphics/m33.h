#pragma once

#include "v3.h"
#include<iostream>
using std::ostream;
using std::istream;

class M33 {
	// overloaded stram insertion operator; cannot be member function
	// if we want to invoke it using cout << someMatrix
	friend ostream& operator<<(ostream &, const M33 &);

	// overloaded stram extraction operator; cannot be member function
	// if we want to invoke it using cout >> someMatrix
	friend istream& operator>>(istream &, M33 &);
private:
  V3 rows[3];
public:
	// constructor from 3 3D vectors
	M33(const V3 &row1, const V3 &row2, const V3 &row3);
	// default constructor
	M33();

	// copy constructor
	// not really needed in this class since automatic shallow copies from
	// compiler would be enough but it doesn't hurt to implement one as an exercise
	// must receive a reference to prevent infinite recursions
	M33(const M33 &matrixToCopy);

	//~M33(); // Default constructor is ok since class doesn't deal with dynamic data

	// overloaded assignment operator
	// Again I'm only implemeting this as exercise. For this class, autogenerated
	// behavior by compiler is probably good enough.
	// const return avoids (m1=m2)=m3.
	// (That is bad not only because is weird syntax but also 
	// because m1 = m2 and m1 = m3 but m2 is never set in that case)
	// return reference does allow for cascaded assignments such as m1 = m2 = m3
	// (m1.operator=(m2.operator(m3)) is allowed and good syntax
	const M33& operator=(const M33 &right);

	// access to rows for reading and writing
	V3& operator[](int i);
    
	// multiplication with 3D vector
	const V3 operator*(const V3 &vector) const;

	// multiplication with another matrix
	const M33 operator*(const M33 &matrix) const;

	// function to get column
	V3 getColumn(int j) const;

	// function to set column
	void setColumn(const V3 &column, int j);

	// get this matrix inverted 
	M33 getInverted(void) const;

	// set this matrix into inverted version
	void setInverted(void);

	// transposition
	M33 getTranspose(void) const;

	// functions to set matrix as rotation about principal axis by theta degrees
	void setRotationAboutX(float theta);
	void setRotationAboutY(float theta);
	void setRotationAboutZ(float theta);
};