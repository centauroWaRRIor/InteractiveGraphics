#include <iostream>
using std::cerr;
using std::endl;
using std::ios;
#include <fstream>
using std::ifstream;
#include "TMesh.h"

TMesh::TMesh():	
	vertsN(0),
	trisN(0),
	verts(nullptr),
	cols(nullptr),
	tris(nullptr)
{

}

TMesh::TMesh(const char * fname)
{
	loadBin(fname);
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
void TMesh::createTetrahedronMesh(V3 *_verts, V3 *_cols)
{
	this->cleanUp();

	vertsN = 4;
	// allocate vertices array
	verts = new V3[vertsN];
	// allocate colors array
	cols = new V3[vertsN];

	// copy data from client
	for (int i = 0; i < 4; i++) {
		verts[i] = _verts[i];
		cols[i] = _cols[i];
	}

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
	trisN = 0;
	vertsN = 0;
}

TMesh::~TMesh()
{
	cleanUp();
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

	delete[]tcs;
	delete[]normals;
}

