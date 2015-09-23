#pragma once

#include "v3.h"

// Implements a triangle mesh class that stores shared vertices and triangle 
// connectivity data.
class TMesh {
private:
	V3 *verts, *cols; // verices and colors arrays
	int vertsN; // number of vertices
	unsigned int *tris; // triangle indices array of size trisN*3
	int trisN; // number of triangle (not number of indices in array)
	void cleanUp(void); // helper function for destructor
public:	
	// empty constructor
	TMesh();
	// constructor out of bin file
	TMesh(const char *fname);
	// destructor
	~TMesh();

	// loads triangle mesh from binary file
	void loadBin(const char *fname);

	// get number of unique vertices in this triangle mesh
	int getVertsN(void) const { return vertsN; }
	// get number of triangles in this triangle mesh
	int getTrisN(void) const { return trisN; }
	// returns vertex at index i or zero vector when initialized
	V3 getVertex(int i) const;
	// returns vertex color at index i or zero vector when initialized
	V3 getVertexColor(int i) const;
	// returns triangle index at index i or -1 when initialized
	int getTriangleIndex(int i) const;

	// constructs a tetrahedron with the given vertices
	void createTetrahedronMesh(V3 *_verts, V3 *_cols);
};