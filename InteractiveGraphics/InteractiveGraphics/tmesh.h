#pragma once
#include "v3.h"
#include "ppc.h"
#include "aabb.h"
#include "texture.h"
#include "sw_framebuffer.h"

// Implements a triangle mesh class that stores shared vertices and triangle 
// connectivity data.
class TMesh {
private:
	V3 *verts; // verices
	// leverage vertex shared triangles to avoid paying for projection
	// more than once:
	V3 *projVerts; // projected vertices
	bool *isVertProjVis; // quickly look up vertex projection status 
	V3 *cols; // colors arrays
	V3 *normals;
	float *tcs; // texture coorindates array (s,t)'s
	int vertsN; // number of vertices
	unsigned int *tris; // triangle indices array of size trisN*3
	int trisN; // number of triangle (not number of indices in array)
	AABB *aabb; // keeps track of current axis aligned box

	// optional hardware rendering support with VAO (VBOs)
	bool isHwSupportEnabled;
	GLuint vertexBuffer[4] = { 0, 0, 0, 0 };
	GLuint indexBuffer;
	GLuint vao; // vertex array object

	void cleanUp(void); // helper function for destructor
	AABB computeAABB(void) const; // computes a bounding box of the centers
	void projectVertices(const PPC &ppc); // optimization: project each vertex only once
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

	// drawing functionality using SW Framebuffer

	// draws the triangle mesh vertices as dots
	void drawVertexDots(SWFrameBuffer &fb, const PPC &ppc, float dotSize);
	// draws triangle mesh in wireframe mode
	void drawWireframe(SWFrameBuffer &fb, const PPC &ppc);
	// draws triangle mesh in filled mode using a single color
	void drawFilledFlat(SWFrameBuffer &fb, const PPC &ppc, unsigned int color);
	// draws triangle mesh in filled mode using screen space interpolation of colors
	void drawFilledFlatBarycentric(SWFrameBuffer &fb, const PPC &ppc);
	// draws triangle mesh in filled mode using model space interpolation of colors
	void drawFilledFlatPerspCorrect(SWFrameBuffer &fb, const PPC &ppc);
	// draws triangle mesh in texture mode using model space for s,t linear interpolation 
	// and screen space for depth linear interpolation
	void drawTextured(SWFrameBuffer &fb, const PPC &ppc, const Texture &texture);
	// same as drawTextured but fragments are discarded based on alpha value. If animated = true
	// then this function iterates over the sprite atlas texture using time as an input
	void drawSprite(SWFrameBuffer &fb, const PPC &ppc, const Texture &texture,
		unsigned int subSIndex = 0, unsigned int subTIndex = 0,
		unsigned int subSTotal = 1, unsigned int subTTotal = 1);
	// draws triangle mesh in lit mode using screen space interpolation for 1/w and model
	// space interpolation for color and s,t parameters. Uses texture if available and uses
	// texture maps if requested.
	void drawLit(
		SWFrameBuffer &fb, 
		const PPC &ppc, 
		const Light &light,
		const LightProjector *const lightProj = nullptr,
		const Texture *const texture = nullptr,
		bool isColorsOn = false,
		bool isShadowMapOn = false,
		bool isLightProjOn = false);
	// draws triangle mesh in filled mode using a single color and depth 1/w in screen
	// coordinates mainly for shadow mapping purposes
	void drawFilledFlatWithDepth(SWFrameBuffer &fb, const PPC &ppc, unsigned int color);
	// draws triangle mesh in stealth mode to support David Copperfiled magic trick
	void drawStealth(
		SWFrameBuffer &fb,
		const PPC &ppc,
		const Light &light,
		const LightProjector &lightProj,
		const Texture *const texture,
		bool isLightOn,
		bool isTexturedOn);
	// draws triangle mesh using reflections with an environment map as if the mesh
	// was constructed out of a highlhy reflective material such as metal.
	// TODO: Add a light to generate specular highlights
	void drawReflective(
		CubeMap &cubeMap,
		SWFrameBuffer &fb,
		const PPC &ppc,
		const Texture *const texture = nullptr,
		bool isColorsOn = false);
	// draws triangle mesh using refractions with an environment map as if the mesh
	// was constructed out of a glass like material
	void drawRefractive(
		float nl, float nt, // nl = refractive index for medium, nt = refractive index of TMesh material
		CubeMap &cubeMap,
		SWFrameBuffer &fb,
		const PPC &ppc,
		const Texture *const texture = nullptr,
		bool isColorsOn = false);

	// hardware accelerated drawing support (optional)
	void hwGLFixedPiepelineDraw(void) const;
	void hwGLVertexArrayObjectDraw(void) const;
	void createGLVertexArrayObject(void);
	bool getIsGLVertexArrayObjectCreated(void) const;
	bool getIsTexCoordsAvailable(void) const;
	void disableTexCoords(void);

	// following copy functions assume destination array is large enough
	void copyNVerts(float * const destination, unsigned int nVerts) const;
	void copyNColors(float * const destination, unsigned int nColors) const;
	void copyNTexCoords(float * const destination, unsigned int nTexCoordsPairs) const;

	// rotate about axis
	void rotateAboutAxis(const V3 &aO, const V3 &adir, float theta);

	// get this triangle mesh's AABB
	AABB getAABB(void) const { return computeAABB(); }
	// draw this triangle mesh's AABB
	void drawAABB(
		SWFrameBuffer &fb,
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
	
	// constructs a tetrahedron for testing purposes
	void createTetrahedronTestMesh(void);
	// creates a quad for texture testing purposes
	void createQuadTMesh(bool isTiling);
	void createQuadTMesh(
		const V3 &p1, 
		const V3 &p2, 
		const V3 &p3, 
		const V3 &p4, 
		bool isTiling);

	// small static utitlities
	// self note: because they are static no need for object instance.
	// To call these just do TMesh::comppute3DTriangleArea(...)
	static float compute3DTriangleArea(const V3 & v1, const V3 & v2, const V3 & v3)
	{
		V3 areaCrossProduct = (v2 - v1) ^ (v3 - v1);
		float area = areaCrossProduct.length() / 2;
		return area;
	}

	static float compute2DTriangleArea(V3 v1, V3 v2, V3 v3)
	{
		// take 3D triangle and ignore z-component
		v1[2] = 0.0f;
		v2[2] = 0.0f;
		v3[2] = 0.0f;
		return compute3DTriangleArea(v1, v2, v3);
	}
};