#include <iostream>
using std::cerr;
using std::endl;
using std::ios;
#include <fstream>
using std::ifstream;
#include "TMesh.h"

const float epsilonMinArea = 0.01f;

TMesh::TMesh():	
	vertsN(0),
	trisN(0),
	verts(nullptr),
	cols(nullptr),
	tris(nullptr),
	aabb(nullptr)
{

}

TMesh::TMesh(const char * fname)
{
	loadBin(fname);
}

TMesh::~TMesh()
{
	cleanUp();
}

void TMesh::cleanUp(void)
{
	if (verts) {
		delete[] verts;
		verts = nullptr;
	}
	if (cols) {
		delete[] cols;
		cols = nullptr;
	}
	if (tris) {
		delete[] tris;
		tris = nullptr;
	}
	if (aabb) {
		delete aabb;
		aabb = nullptr;
	}
	trisN = 0;
	vertsN = 0;
}

V3 TMesh::getVertex(int i) const {
	if (verts && i < vertsN) {
		return V3(verts[i]);
	}
	else {
		cerr << "ERROR: Attempting to get invalid vertex from TMesh. Zero vector returned..." << endl;
		return V3();
	}
}

V3 TMesh::getVertexColor(int i) const {
	if (cols && i < vertsN) {
		return V3(cols[i]);
	}
	else {
		cerr << "ERROR: Attempting to get invalid vertex color from TMesh. Zero vector returned..." << endl;
		return V3();
	}
}

int TMesh::getTriangleIndex(int i) const {
	if (tris && i < (3 * trisN)) {
		return tris[i];
	}
	else {
		cerr << "ERROR: Attempting to get invalid triangle index from TMesh. -1 returned..." << endl;
		return -1;
	}
}

// constructs a tetrahedron with the given vertices
void TMesh::createTetrahedronTestMesh(void)
{
	this->cleanUp();

	vertsN = 4;
	// allocate vertices array
	verts = new V3[vertsN];
	// allocate colors array
	cols = new V3[vertsN];

	// following vertices define a sample tetrahedron
	verts[0] = V3(0.0f, 35.0f, -100.0f);
	cols[0] = V3(0.0f, 0.0f, 0.0f);
	verts[1] = V3(-20.0f, -15.0f, -100.0f);
	cols[1] = V3(1.0f, 0.0f, 0.0f);
	verts[2] = V3(7.0f, -15.0f, -50.0f);
	cols[2] = V3(0.0f, 1.0f, 0.0f);
	verts[3] = V3(40.0f, -15.0f, -130.0f);
	cols[3] = V3(0.0f, 0.0f, 1.0f);

	trisN = 4;

	// allocate triangle indices array
	tris = new unsigned int[3 * trisN];

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 2;

	tri++;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 2;
	tris[3 * tri + 2] = 3;

	tri++;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 1;

	tri++;
	tris[3 * tri + 0] = 1;
	tris[3 * tri + 1] = 3;
	tris[3 * tri + 2] = 2;

	// compute aabb
	aabb = new AABB(computeAABB());
}

void TMesh::loadBin(const char * fname)
{
	// clean in case it had other stuff already loaded
	this->cleanUp();

	ifstream ifs(fname, ios::binary);
	if (ifs.fail()) {
		cerr << "INFO: cannot open triangle mesh bin file: " << fname << endl;
		return;
	}

	// reads numbers of vertices
	ifs.read((char*)&vertsN, sizeof(int));
	char yn;
	ifs.read(&yn, 1); // file must always have xyz
	if (yn != 'y') {
		cerr << "INTERNAL ERROR: there should always be vertex xyz data" << endl;
		return;
	}
	// allocate space for the vertex data
	verts = new V3[vertsN];

	// reads whether or not there is color info in the file
	ifs.read(&yn, 1); // cols 3 floats
	if (yn == 'y') {
		// allocate space for colors
		cols = new V3[vertsN];
	}

	// reads whether or not there is normal info in the file
	ifs.read(&yn, 1); // normals 3 floats
	V3 *normals = nullptr; // TMesh doesn't store normals for now
	//if (normals)
	//	delete [] normals;
	//normals = nullptr;
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	// reads whether or not there is texture info in the file
	ifs.read(&yn, 1); // texture coordinates 2 floats
	float *tcs = nullptr; // don't have texture coordinates for now
	//if (tcs)
		//delete [] tcs;
	//tcs = nullptr;
	if (yn == 'y') {
		tcs = new float[vertsN * 2];
	}

	ifs.read((char*)verts, vertsN * 3 * sizeof(float)); // load verts

	if (cols) {
		ifs.read((char*)cols, vertsN * 3 * sizeof(float)); // load cols
	}

	if (normals)
		ifs.read((char*)normals, vertsN * 3 * sizeof(float)); // load normals

	if (tcs)
		ifs.read((char*)tcs, vertsN * 2 * sizeof(float)); // load texture coordinates

	// read triangles indices
	ifs.read((char*)&trisN, sizeof(int));
	tris = new unsigned int[trisN * 3];
	ifs.read((char*)tris, trisN * 3 * sizeof(unsigned int)); // read tiangles

	ifs.close(); // destructor closes file

	cerr << "INFO: loaded " << vertsN << " verts, " << trisN << " tris from " << endl << "      " << fname << endl;
	cerr << "      xyz " << ((cols) ? "rgb " : "") << ((normals) ? "nxnynz " : "") << ((tcs) ? "tcstct " : "") << endl;

	// recompute AABB
	aabb = new AABB(computeAABB());

	delete[]tcs;
	delete[]normals;
}

void TMesh::drawFilledFlat(FrameBuffer &fb, const PPC &ppc, unsigned int color) const
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawWireframe() command was aborted." << endl;
		return;
	}

	// Draw filled triangles with single color (provided)
	for (int tri = 0; tri < trisN; tri++) {
		V3 currvs[3];
		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];
		V3 currcols[3];
		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// project triangle vertices using camera
		V3 projV1, projV2, projV3;
		bool isVisible = ppc.project(currvs[0], projV1);
		isVisible &= ppc.project(currvs[1], projV2);
		isVisible &= ppc.project(currvs[2], projV3);

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(projV1, projV2, projV3);
			if (projTriangleArea > epsilonMinArea) {

				float u[3], v[3];
				u[0] = projV1.getX();
				u[1] = projV2.getX();
				u[2] = projV3.getX();
				v[0] = projV1.getY();
				v[1] = projV2.getY();
				v[2] = projV3.getY();
				fb.draw2DFlatTriangle(u, v, color);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawFilledFlatBarycentric(FrameBuffer &fb, const PPC &ppc) const {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawWireframe() command was aborted." << endl;
		return;
	}

	// Draw filled triangles with color linearly interpolated 
	// in screen space.
	for (int tri = 0; tri < trisN; tri++) {
		V3 currvs[3];
		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];
		V3 currcols[3];
		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// project triangle vertices using camera
		V3 projV1, projV2, projV3;
		bool isVisible = ppc.project(currvs[0], projV1);
		isVisible &= ppc.project(currvs[1], projV2);
		isVisible &= ppc.project(currvs[2], projV3);
		if (isVisible) {

			// The discriminat of the matrix used in screen space interpolation 
			// is the area of the projected triangle. When that is 0, the matrix 
			// cannot be inverted, as one cannot compute the screen space variation
			// for a triangle with collinear or coincident vertices.
			// Therefore, rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(projV1, projV2, projV3);

			if (projTriangleArea > epsilonMinArea) {

				fb.draw2DFlatBarycentricTriangle(
					projV1, currcols[0],
					projV2, currcols[1],
					projV3, currcols[2]);
			}
			else 
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::draw3DSegment(
	const V3 &v0, const V3 &c0, 
	const V3 &v1, const V3 &c1, 
	FrameBuffer &fb, const PPC &ppc) const 
{
	V3 projv0, projv1;
	if (!ppc.project(v0, projv0))
		return;
	if (!ppc.project(v1, projv1))
		return;
	fb.draw2DSegment(projv0, c0, projv1, c1);
}

void TMesh::drawWireframe(FrameBuffer &fb, const PPC &ppc) const {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawWireframe() command was aborted." << endl;
		return;
	}

	// Draw vertices connections as line segments
	for (int tri = 0; tri < trisN; tri++) {
		V3 currvs[3];
		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];
		V3 currcols[3];
		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];
		// draw edges between vertices of this triangle
		// e1 = 0,1  e2 = 1,2  e3 = 2,0 (hence the %3)
		for (int ei = 0; ei < 3; ei++) {
			draw3DSegment(
				currvs[ei], currcols[ei],
				currvs[(ei + 1) % 3], currcols[(ei + 1) % 3], 
				fb, ppc);
		}
	}
}

void TMesh::drawVertexDots(FrameBuffer &fb,const PPC &ppc, float dotSize) const {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawVertexDots() command was aborted." << endl;
		return;
	}

	// Draw vertices as cricles
	for (int vi = 0; vi < vertsN; vi++) {
		V3 projP;
		if (!ppc.project(verts[vi], projP))
			continue;
		fb.draw2DCircleIfCloser(projP, dotSize, cols[vi]);
	}
}

void TMesh::rotateAboutAxis(const V3 &aO, const V3 &adir, float theta)
{
	for (int vi = 0; vi < vertsN; vi++) {
		
		verts[vi].rotateThisPointAboutAxis(aO, adir, theta);
	}
	// rotate the AABB as well

}

AABB TMesh::computeAABB(void) const
{
	if (vertsN == 0) {
		cerr << "ERROR: Attempted to compute AABB from an empty mesh. "
			<< "Returning empty AABB" << endl;
		return AABB(V3());
	}
	else {
		AABB ret(verts[0]);
		for (int vi = 0; vi < vertsN; vi++) {
			ret.AddPoint(verts[vi]);
		}
		return ret;
	}
}

void TMesh::drawAABB(FrameBuffer & fb, const PPC & ppc, unsigned int colorNear, unsigned int colorFar) const
{
	V3 cnear;
	cnear.setFromColor(colorNear);
	V3 cfar;
	cfar.setFromColor(colorFar);

	if (vertsN == 0) {
		cerr << "ERROR: Attempted to draw the AABB of an empty mesh. "
			<< "Aborting..." << endl;
	}

	V3 p0, p1;

	V3 corner1 = aabb->getFristCorner();
	V3 corner2 = aabb->getSecondCorner();

	p0 = V3(corner1[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner1[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner1[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner2[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner2[1], corner2[2]);
	draw3DSegment(p0, cnear, p1, cnear, fb, ppc);

	p0 = V3(corner1[0], corner2[1], corner1[2]);
	p1 = V3(corner1[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner1[2]);
	p1 = V3(corner2[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner1[2]);
	p1 = V3(corner2[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner1[2]);
	p1 = V3(corner1[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cfar, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner2[1], corner2[2]);
	p1 = V3(corner1[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner1[0], corner1[1], corner2[2]);
	p1 = V3(corner1[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner1[1], corner2[2]);
	p1 = V3(corner2[0], corner1[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

	p0 = V3(corner2[0], corner2[1], corner2[2]);
	p1 = V3(corner2[0], corner2[1], corner1[2]);
	draw3DSegment(p0, cnear, p1, cfar, fb, ppc);

#if 0
	4		7

0-------3
|	5	|	6
|		|
1-------2
#endif

}

V3 TMesh::getCenter(void) const
{
	if (vertsN == 0) {
		cerr << "ERROR: Attempted to get center from an empty mesh. "
			<< "Returning zero vector" << endl;
		return V3();
	}

	V3 ret(0.0f, 0.0f, 0.0f);
	for (int vi = 0; vi < vertsN; vi++) {
		ret = ret + verts[vi];
	}
	ret = ret / (float)vertsN;
	return ret;
}

void TMesh::scale(float scaleFactor)
{
	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi] * scaleFactor;
	}
	// recompute AABB
	delete aabb;
	aabb = nullptr;
	aabb = new AABB(computeAABB());
}

void TMesh::translate(const V3 & translationVector)
{
	for (int vi = 0; vi < vertsN; vi++) {
		verts[vi] = verts[vi] + translationVector;
	}
	// recompute AABB
	delete aabb;
	aabb = nullptr;
	aabb = new AABB(computeAABB());
}

void TMesh::setToFitAABB(const AABB & aabb)
{
	V3 corner1 = aabb.getFristCorner();
	V3 corner2 = aabb.getSecondCorner();

	// 1) figure out the scaling factor:

	// compute length of aabb's diagonal
	V3 aabbDiag(corner2 - corner1);
	float aabbDiagLength = aabbDiag.length();
	
	// compute length of this aabb's diagonal
	AABB thisAABB(computeAABB());
	V3 thisCorner1 = thisAABB.getFristCorner();
	V3 thisCorner2 = thisAABB.getSecondCorner();
	V3 thisAabbDiag(thisCorner2 - thisCorner1);
	float thisAabbDiagLength = thisAabbDiag.length();
	
	// extract scale factor
	float scaleFactor = aabbDiagLength / thisAabbDiagLength;

	// 2) apply scale
	scale(scaleFactor);

	// 3) with new scale in figure out translation vector

	// find AABB center
	V3 aabbCenter = corner1 + ((corner2 - corner1) * (0.5f));

	// find this Tmesh center
	V3 thisCenter = getCenter();

	// find the translation vector from centroid to 
	// center of AABB
	V3 translationVector = aabbCenter - thisCenter;

	// 4) apply translation
	translate(translationVector);
}

