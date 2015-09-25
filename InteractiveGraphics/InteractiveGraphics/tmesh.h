#pragma once
#include "v3.h"
#include "ppc.h"
#include "aabb.h"
#include "framebuffer.h"

// Implements a triangle mesh class that stores shared vertices and triangle 
// connectivity data.
class TMesh {
private:
	V3 *verts, *cols; // verices and colors arrays
	int vertsN; // number of vertices
	unsigned int *tris; // triangle indices array of size trisN*3
	int trisN; // number of triangle (not number of indices in array)

	void cleanUp(void); // helper function for destructor
	AABB computeAABB(void) const; // computes a bounding box of the centers
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

	// drawing functionality
	// draws triangle mesh in filled mode using a single color
	void drawFilledFlat(FrameBuffer &fb, const PPC &ppc, unsigned int color) const;
	// draws triangle mesh in filled mode using screen space interpolation of colors
	void drawFilledFlatBarycentric(FrameBuffer &fb, const PPC &ppc) const;
	// draws triangle mesh in wireframe mode
	void drawWireframe(FrameBuffer &fb, const PPC &ppc) const;
	// draws the triangle mesh vertices as dots
	void drawVertexDots(FrameBuffer &fb, const PPC &ppc, float dotSize) const;

	// get this triangle mesh's AABB
	AABB getAABB(void) const { return computeAABB(); }
	// draw this triangle mesh's AABB
	void drawAABB(
		FrameBuffer &fb,
		const PPC &ppc,
		unsigned int colorNear,
		unsigned int colorFar) const;
	// returns center of mass (also called centroid) of vertices
	V3 getCenter(void) const;

	// scale this triangle mesh vertices
	void scale(float scaleFactor);
	// translate this triangle mesh vertices by translation vector
	void translate(const V3 &translationVector);
	// places the centroid at a given position and scales to given AABB size
	void setToFitAABB(const AABB &aabb);
	
	// constructs a tetrahedron with the given vertices
	void createTetrahedronMesh(V3 *_verts, V3 *_cols);
};