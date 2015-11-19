#include <iostream>
using std::cerr;
using std::endl;
using std::ios;
#include <fstream>
using std::ifstream;
#include <GL\glew.h>
#include "TMesh.h"
const float epsilonMinArea = 0.1f;

TMesh::TMesh():	
	vertsN(0),
	trisN(0),
	verts(nullptr),
	projVerts(nullptr),
	isVertProjVis(nullptr),
	cols(nullptr),
	tcs(nullptr),
	normals(nullptr),
	tris(nullptr),
	aabb(nullptr),
	indexBuffer(0),
	vao(0),
	isHwSupportEnabled(false)
{

}

TMesh::TMesh(const char * fname)
{
	loadBin(fname);
}

TMesh::~TMesh()
{
	cleanUp();
	if (isHwSupportEnabled) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vertexBuffer[0]);
		glDeleteBuffers(1, &vertexBuffer[1]);
		// replace with glDeleteBuffers(4, vertexBuffer) when using the 4 slots
		glDeleteBuffers(1, &indexBuffer);
	}
}

void TMesh::cleanUp(void)
{
	if (verts) {
		delete[] verts;
		delete[] projVerts;
		delete[] isVertProjVis;
		verts = nullptr;
		projVerts = nullptr;
		isVertProjVis = nullptr;
	}
	if (cols) {
		delete[] cols;
		cols = nullptr;
	}
	if (normals) {
		delete[] normals;
		normals = nullptr;
	}
	if (tcs) {
		delete[] tcs;
		tcs = nullptr;
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
	projVerts = new V3[vertsN];
	isVertProjVis = new bool[vertsN];
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

void TMesh::createQuadTestTMesh(bool isTiling)
{
	this->cleanUp();

	vertsN = 4;
	// allocate vertices array
	verts = new V3[vertsN];
	projVerts = new V3[vertsN];
	isVertProjVis = new bool[vertsN];
	// allocate colors array
	cols = new V3[vertsN];
	// allocate tex coords array
	tcs = new float[vertsN * 2];
	// allocate normals array
	normals = new V3[vertsN];

	// following vertices define a sample tetrahedron
	verts[0] = V3(0.0f, 0.0f, 0.0f);
	cols[0] = V3(0.0f, 0.0f, 0.0f);
	normals[0] = V3(0.0f, 0.0f, 1.0f);
	tcs[0] = 0.0;
	tcs[1] = 0.0;
	verts[1] = V3(40.0f, 0.0f, 0.0f);
	cols[1] = V3(1.0f, 0.0f, 0.0f);
	normals[1] = V3(0.0f, 0.0f, 1.0f);
	tcs[2] = isTiling ? 2.0f : 1.0f;
	tcs[3] = 0.0;
	verts[2] = V3(40.0f, 40.0f, 0.0f);
	cols[2] = V3(0.0f, 1.0f, 0.0f);
	normals[2] = V3(0.0f, 0.0f, 1.0f);
	tcs[4] = isTiling ? 2.0f : 1.0f;
	tcs[5] = isTiling ? 2.0f : 1.0f;
	verts[3] = V3(0.0f, 40.0f, 0.0f);
	cols[3] = V3(0.0f, 0.0f, 1.0f);
	normals[3] = V3(0.0f, 0.0f, 1.0f);
	tcs[6] = 0.0;
	tcs[7] = isTiling ? 2.0f : 1.0f;

	trisN = 2;

	// allocate triangle indices array
	tris = new unsigned int[3 * trisN];

	int tri = 0;
	tris[3 * tri + 0] = 0;
	tris[3 * tri + 1] = 1;
	tris[3 * tri + 2] = 3;

	tri++;
	tris[3 * tri + 0] = 3;
	tris[3 * tri + 1] = 1;
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
	projVerts = new V3[vertsN];
	isVertProjVis = new bool[vertsN];

	// reads whether or not there is color info in the file
	ifs.read(&yn, 1); // cols 3 floats
	if (yn == 'y') {
		// allocate space for colors
		cols = new V3[vertsN];
	}

	// reads whether or not there is normal info in the file
	ifs.read(&yn, 1); // normals 3 floats
	if (yn == 'y') {
		normals = new V3[vertsN];
	}

	// reads whether or not there is texture info in the file
	ifs.read(&yn, 1); // texture coordinates 2 floats
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
}


void TMesh::drawWireframe(SWFrameBuffer &fb, const PPC &ppc) {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawWireframe() command was aborted." << endl;
		return;
	}

	// Draw vertices connections as line segments
	V3 currcols[3];
	V3 projV[3];
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current projected triangle vertices
		projV[0] = projVerts[tris[3 * tri + 0]];
		projV[1] = projVerts[tris[3 * tri + 1]];
		projV[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// draw edges between vertices of this triangle
		// e1 = 0,1  e2 = 1,2  e3 = 2,0 (hence the %3)
		for (int ei = 0; ei < 3; ei++) {

			if(isVisible)
				fb.draw2DSegment(
					projV[ei],
					currcols[ei], 
					projV[(ei + 1) % 3],
					currcols[(ei + 1) % 3]);
		}
	}
}

void TMesh::drawVertexDots(SWFrameBuffer &fb, const PPC &ppc, float dotSize) {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawVertexDots() command was aborted." << endl;
		return;
	}

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int vi = 0; vi < vertsN; vi++) {
		if (!isVertProjVis[vi])
			continue;
		// Draw vertices as cricles
		fb.draw2DCircleIfCloser(projVerts[vi], dotSize, cols[vi]);
	}
}

void TMesh::drawFilledFlat(SWFrameBuffer &fb, const PPC &ppc, unsigned int color)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawFilledFlat() command was aborted." << endl;
		return;
	}

	// draw filled triangles with single color (provided)
	V3 tProjVerts[3];
	bool isVisible;
	
	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {
		
		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];
		
		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {
				fb.draw2DFlatTriangle(tProjVerts, color);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawFilledFlatBarycentric(SWFrameBuffer &fb, const PPC &ppc) {

	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawFilledFlatBarycentric() command was aborted." << endl;
		return;
	}

	// Draw filled triangles with color and depth linearly 
	// interpolated in screen space.
	V3 currcols[3];
	V3 tProjVerts[3];
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		if (isVisible) {

			// The discriminat of the matrix used in screen space interpolation 
			// is the area of the projected triangle. When that is 0, the matrix 
			// cannot be inverted, as one cannot compute the screen space variation
			// for a triangle with collinear or coincident vertices.
			// Therefore, rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				fb.draw2DFlatTriangleScreenSpace(
					tProjVerts, currcols);
			}
			else 
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawFilledFlatPerspCorrect(SWFrameBuffer & fb, const PPC & ppc)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawFilledFlatPerspCorrect() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 Q, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw filled triangles with color and depth linearly 
	// interpolated in model space.
	V3 currvs[3];
	V3 currcols[3];
	V3 tProjVerts[3];
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				Q = VMinC * abc;

				fb.draw2DFlatTriangleModelSpace(
					tProjVerts, currcols, Q);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawTextured(SWFrameBuffer & fb, const PPC & ppc, const Texture & texture)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawTextured() command was aborted." << endl;
		return;
	}

	if (tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in texture mode without specifying s,t's. "
			<< "drawTextured() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// grab current triangle texture coordinates
		sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
		sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
		sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
		tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
		tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
		tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				fb.draw2DTexturedTriangle(
					tProjVerts, currcols,
					sParameters, tParameters,
					perspCorrectMatQ,
					texture);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawSprite(
	SWFrameBuffer & fb, 
	const PPC & ppc, 
	const Texture & texture,
	unsigned int subSIndex, 
	unsigned int subTIndex,
	unsigned int subSTotal,
	unsigned int subTTotal)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawSprite() command was aborted." << endl;
		return;
	}

	if (tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in texture mode without specifying s,t's. "
			<< "drawSprite() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// grab current triangle texture coordinates
		sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
		sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
		sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
		tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
		tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
		tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];

		// shift triangle texture coordinates by subS and subB
		if ((tri % 2) == 0) { // if first triangle of quad
			sParameters[0] += subSIndex * (1 / (float)subSTotal);
			sParameters[1] *= (subSIndex + 1) * (1 / (float)subSTotal);
			sParameters[2] += subSIndex * (1 / (float)subSTotal);
			tParameters[0] += subTIndex * (1 / (float)subTTotal);
			tParameters[1] += subTIndex * (1 / (float)subTTotal);
			tParameters[2] *= (subTIndex + 1) * (1 / (float)subTTotal);
		}
		else { // if second triangle of quad
			sParameters[0] += subSIndex * (1 / (float)subSTotal);
			sParameters[1] *= (subSIndex + 1) * (1 / (float)subSTotal);
			sParameters[2] *= (subSIndex + 1) * (1 / (float)subSTotal);
			tParameters[0] *= (subTIndex + 1) * (1 / (float)subTTotal);
			tParameters[1] += subTIndex * (1 / (float)subTTotal);
			tParameters[2] *= (subTIndex + 1) * (1 / (float)subTTotal);
		}

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				fb.draw2DSprite(
					tProjVerts, currcols,
					sParameters, tParameters,
					perspCorrectMatQ,
					texture);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawLit(
	SWFrameBuffer &fb,
	const PPC &ppc,
	const Light &light,
	const LightProjector *const lightProj,
	const Texture *const texture,
	bool isColorsOn,
	bool isShadowMapOn,
	bool isLightProjOn)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawLit() command was aborted." << endl;
		return;
	}
	else if (normals == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit mode without specifying normals. "
			<< "drawLit() command was aborted." << endl;
		return;
	}
	else if (texture != nullptr && tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit texture mode without specifying s,t's. "
			<< "drawLit() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 currnormals[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];
		
		// grab current normals
		currnormals[0] = normals[tris[3 * tri + 0]];
		currnormals[1] = normals[tris[3 * tri + 1]];
		currnormals[2] = normals[tris[3 * tri + 2]];

		if (texture != nullptr) {
			// grab current triangle texture coordinates
			sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
			sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
			sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
			tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
			tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
			tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];
		}
		else {
			// make something up, they won't be used anyways
			sParameters = V3(0.0f, 1.0f, 0.0f);
			tParameters = V3(0.0f, 1.0f, 1.0f);
		}

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				if (isColorsOn) {
					fb.draw2DLitTriangle(
						currvs, tProjVerts, currcols, currnormals,
						light, perspCorrectMatQ,
						sParameters, tParameters,
						texture,
						isShadowMapOn,
						ppc,
						isLightProjOn,
						lightProj);
				}
				else {
					fb.draw2DLitTriangle(
						currvs, tProjVerts, nullptr, currnormals,
						light, perspCorrectMatQ,
						sParameters, tParameters,
						texture,
						isShadowMapOn,
						ppc,
						isLightProjOn,
						lightProj);
				}
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawFilledFlatWithDepth(SWFrameBuffer & fb, const PPC & ppc, unsigned int color)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawFilledFlatWithDepth() command was aborted." << endl;
		return;
	}

	// draw filled triangles with single color (provided)
	V3 tProjVerts[3];
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {
				fb.draw2DFlatTriangleWithDepth(tProjVerts, color);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawStealth(
	SWFrameBuffer & fb, 
	const PPC & ppc, 
	const Light & light, 
	const LightProjector & lightProj,
	const Texture * const texture, 
	bool isLightOn,
	bool isTexturedOn)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawStealth() command was aborted." << endl;
		return;
	}
	else if (normals == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit mode without specifying normals. "
			<< "drawStealth() command was aborted." << endl;
		return;
	}
	else if (texture != nullptr && tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit texture mode without specifying s,t's. "
			<< "drawStealth() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 currnormals[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// grab current normals
		currnormals[0] = normals[tris[3 * tri + 0]];
		currnormals[1] = normals[tris[3 * tri + 1]];
		currnormals[2] = normals[tris[3 * tri + 2]];

		if (isTexturedOn) {
			// grab current triangle texture coordinates
			sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
			sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
			sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
			tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
			tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
			tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];
		}
		else {
			// make something up, they won't be used anyways
			sParameters = V3(0.0f, 1.0f, 0.0f);
			tParameters = V3(0.0f, 1.0f, 1.0f);
		}

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				fb.draw2DStealthTriangle(
					currvs, tProjVerts, currcols, currnormals,
					light, perspCorrectMatQ,
					isLightOn, isTexturedOn,
					sParameters, tParameters,
					texture,
					ppc,					
					lightProj);
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::projectVertices(const PPC & ppc)
{
	for (int vi = 0; vi < vertsN; vi++) {
		isVertProjVis[vi] = ppc.project(verts[vi], projVerts[vi]);
	}
}

void TMesh::drawReflective(
	CubeMap &cubeMap,
	SWFrameBuffer & fb, 
	const PPC & ppc, 
	const Texture * 
	const texture, 
	bool isColorsOn)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}
	else if (normals == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit mode without specifying normals. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}
	else if (texture != nullptr && tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit texture mode without specifying s,t's. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 currnormals[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// grab current normals
		currnormals[0] = normals[tris[3 * tri + 0]];
		currnormals[1] = normals[tris[3 * tri + 1]];
		currnormals[2] = normals[tris[3 * tri + 2]];

		if (texture != nullptr) {
			// grab current triangle texture coordinates
			sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
			sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
			sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
			tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
			tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
			tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];
		}
		else {
			// make something up, they won't be used anyways
			sParameters = V3(0.0f, 1.0f, 0.0f);
			tParameters = V3(0.0f, 1.0f, 1.0f);
		}

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				if (isColorsOn) {
					fb.draw2DReflectiveTriangle(
						cubeMap, ppc,
						currvs, tProjVerts, currcols, currnormals,
						perspCorrectMatQ,
						sParameters, tParameters,
						texture);
				}
				else {
					fb.draw2DReflectiveTriangle(
						cubeMap, ppc,
						currvs, tProjVerts, nullptr, currnormals,
						perspCorrectMatQ,
						sParameters, tParameters,
						texture);
				}
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::drawRefractive(
	float nl, float nt, 
	CubeMap & cubeMap, 
	SWFrameBuffer & fb, 
	const PPC & ppc, 
	const Texture * const texture, 
	bool isColorsOn)
{
	if ((vertsN == 0) || (trisN < 1)) {
		cerr << "ERROR: Attempted to draw an empty mesh. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}
	else if (normals == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit mode without specifying normals. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}
	else if (texture != nullptr && tcs == nullptr) {
		cerr << "ERROR: Attempted to draw a mesh in lit texture mode without specifying s,t's. "
			<< "drawReflective() command was aborted." << endl;
		return;
	}

	// Matrix used for perspective correct linear 
	// interpolation of raster parameters.
	// (refer to slide 7 of RastParInterp.pdf for the math 
	// derivation of Q matrix)
	M33 perspCorrectMatQ, VMinC, abc;
	abc.setColumn(ppc.getLowerCaseA(), 0);
	abc.setColumn(ppc.getLowerCaseB(), 1);
	abc.setColumn(ppc.getLowerCaseC(), 2);

	// Draw textured triangles with s,t linearly
	// interpolated in model space and depth linearly
	// interpolated in screen space.
	V3 tProjVerts[3];
	V3 currvs[3];
	V3 currcols[3];
	V3 currnormals[3];
	V3 sParameters, tParameters;
	bool isVisible;

	// optimization: project each vertex only once
	projectVertices(ppc);

	for (int tri = 0; tri < trisN; tri++) {

		// grab current triangle vertices
		currvs[0] = verts[tris[3 * tri + 0]];
		currvs[1] = verts[tris[3 * tri + 1]];
		currvs[2] = verts[tris[3 * tri + 2]];

		// grab current projected triangle vertices
		tProjVerts[0] = projVerts[tris[3 * tri + 0]];
		tProjVerts[1] = projVerts[tris[3 * tri + 1]];
		tProjVerts[2] = projVerts[tris[3 * tri + 2]];
		isVisible = isVertProjVis[tris[3 * tri + 0]];
		isVisible &= isVertProjVis[tris[3 * tri + 1]];
		isVisible &= isVertProjVis[tris[3 * tri + 2]];

		// grab current triangle vertex colors
		currcols[0] = cols[tris[3 * tri + 0]];
		currcols[1] = cols[tris[3 * tri + 1]];
		currcols[2] = cols[tris[3 * tri + 2]];

		// grab current normals
		currnormals[0] = normals[tris[3 * tri + 0]];
		currnormals[1] = normals[tris[3 * tri + 1]];
		currnormals[2] = normals[tris[3 * tri + 2]];

		if (texture != nullptr) {
			// grab current triangle texture coordinates
			sParameters[0] = tcs[tris[3 * tri + 0] * 2 + 0];
			sParameters[1] = tcs[tris[3 * tri + 1] * 2 + 0];
			sParameters[2] = tcs[tris[3 * tri + 2] * 2 + 0];
			tParameters[0] = tcs[tris[3 * tri + 0] * 2 + 1];
			tParameters[1] = tcs[tris[3 * tri + 1] * 2 + 1];
			tParameters[2] = tcs[tris[3 * tri + 2] * 2 + 1];
		}
		else {
			// make something up, they won't be used anyways
			sParameters = V3(0.0f, 1.0f, 0.0f);
			tParameters = V3(0.0f, 1.0f, 1.0f);
		}

		if (isVisible) {

			// Rasterizer should reject triangles whose screen footprint is very small.
			float projTriangleArea = compute2DTriangleArea(
				tProjVerts[0],
				tProjVerts[1],
				tProjVerts[2]);

			if (projTriangleArea > epsilonMinArea) {

				// build model space linear interpolation for s and t
				VMinC.setColumn(currvs[0] - ppc.getEyePoint(), 0);
				VMinC.setColumn(currvs[1] - ppc.getEyePoint(), 1);
				VMinC.setColumn(currvs[2] - ppc.getEyePoint(), 2);
				VMinC.setInverted();
				perspCorrectMatQ = VMinC * abc;

				if (isColorsOn) {
					fb.draw2DRefractiveTriangle(
						nl, nt, cubeMap, ppc,
						currvs, tProjVerts, currcols, currnormals,
						perspCorrectMatQ,
						sParameters, tParameters,
						texture);
				}
				else {
					fb.draw2DRefractiveTriangle(
						nl, nt, cubeMap, ppc,
						currvs, tProjVerts, nullptr, currnormals,
						perspCorrectMatQ,
						sParameters, tParameters,
						texture);
				}
			}
			else
				cerr << "WARNING: Triangle screen footprint is stoo small, discarding..." << endl;
		}
	}
}

void TMesh::hwGLFixedPiepelineDraw(void) const
{
	// Draw using old OpenGL fixed pipeline functionlity API (this is deprecated functionality)
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, (float*)verts);
	glColorPointer(3, GL_FLOAT, 0, (float*)cols);

	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, tris);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

void TMesh::hwGLVertexArrayObjectDraw(void) const
{
	// Draw using new OpenGL API (VBOs as opposed to deprecated functionality)
	glBindVertexArray(vao);

	//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer); // not needed ?
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer); // not needed ?

	//The is the number of indices. 3 indices needed to make a single triangle
	glDrawElements(GL_TRIANGLES, 3 * trisN, GL_UNSIGNED_INT, NULL);
}

void TMesh::createGLVertexArrayObject(void)
{
	// A Vertex Array Object(VAO) is an object which contains one or more Vertex Buffer Objects 
	// and is designed to store the information for a complete rendered object.
	
	// A Vertex Buffer Object(VBO) is a memory buffer in the high speed memory of your video card
	// designed to hold information about vertices. VBOs could describes the coordinates of our vertices
	// and describes the color associated with each vertex. VBOs can also store information such as normals, 
	// texcoords, indices, etc.

	// VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// VBOs
	glGenBuffers(4, vertexBuffer);
	// vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[0]);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * 3) * vertsN, (float *)verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);// todo: comment on meaning of each parameter here
	glEnableVertexAttribArray(0);
	// color data
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer[1]);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(float) * 3) * vertsN, (float *)cols, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);// todo: comment on meaning of each parameter here
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * trisN, tris, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	isHwSupportEnabled = true;
}

bool TMesh::getIsGLVertexArrayObjectCreated(void) const
{
	return isHwSupportEnabled;
}

void TMesh::rotateAboutAxis(const V3 &aO, const V3 &adir, float theta)
{
	for (int vi = 0; vi < vertsN; vi++) {
		// rotate TMesh's vertices
		verts[vi].rotateThisPointAboutAxis(aO, adir, theta);
		// rotate TMesh's normals if available
		if (normals) {
			normals[vi].rotateThisVectorAboutDirection(adir, theta);
		}
	}
	// recompute AABB
	delete aabb;
	aabb = nullptr;
	aabb = new AABB(computeAABB());
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

void TMesh::drawAABB(SWFrameBuffer & fb, const PPC & ppc, unsigned int colorNear, unsigned int colorFar) const
{


	if (vertsN == 0) {
		cerr << "ERROR: Attempted to draw the AABB of an empty mesh. "
			<< "Aborting..." << endl;
	}
	else {
		this->aabb->draw(fb, ppc, colorNear, colorFar);
	}
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

	// recompute and store new AABB
	delete this->aabb;
	this->aabb = nullptr;
	this->aabb = new AABB(computeAABB());
}

