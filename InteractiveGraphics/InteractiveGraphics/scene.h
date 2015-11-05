#pragma once

#include <string>
using std::string;
#include "gui.h"
#include "framebuffer.h"
#include "tmesh.h"

// Only those function that actually require a TMesh slot are 
// registered here
enum class Scenes { 
	DBG, 
	A1, 
	A2, 
	CAMLERP,
	CAMCONTROL,
	TEXTURE,
	A3,
	SPRITETEST,
	SHADOWTEST,
	TEXPROJTEST,
	A4,
	A4EXTRA,
	REFLECTEST,
	REFRACTEST};

enum class DrawModes { 
	DOTS, 
	WIREFRAME, 
	FLAT, 
	SCREENSCAPELERP, 
	MODELSPACELERP, 
	TEXTURE,
	LIT};

class Scene {
private:
	FrameBuffer *fb; // SW framebuffer
	FrameBuffer *fbAux; // currently used for rendering a FB as a 3D point cloud
	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	Light *light; // light used to render models in lit mode
	LightProjector *lightProjector; // light used to project textures
	int tmsN; // how many TMeshes there are
	TMesh **tms; // an array of pointers to TMesh objects
	Texture **texObjects; // and array of pointers to Texture objects

	// use for camera dolly with the mouse in REFLECTEST
	int mouseDeltaX, mouseDeltaY;

	Scenes currentScene; // used for keyboard callback to invokate the correct redraw
	DrawModes currentDrawMode; // controls how to draw current scene
	// helps initializnig the different demo functions

	bool isA2Init, isDGBInit, isTestCamControlsInit,
		isTextureInit, isA3Init, isSpriteTestInit, 
		isTestBilTexLookupInit, isShadowMapTestInit,
		isTexProjectTestInit, isA4DemoInit, isA4DemoExtraInit,
		isTestCMReflectInit, isTestCMRefractInit;
	
	static const int u0, v0; // initial window coordinates

	// these two cameras are used as interpolation helpers
	PPC *ppcLerp0;
	PPC *ppcLerp1;

	// utlitliy function to retrieve time and date as a string
	string retrieveTimeDate(void) const;
	// utility function to centralize all triangle mesh drawing options
	void drawTMesh(
		TMesh &tMesh, 
		FrameBuffer &fB, 
		const PPC &ppc,
		bool isAABBDrawn,
		bool isShadowsEnabled = false,
		bool isLightProjEnabled = false);
	// utility function to clean in between different scene demos
	// and avoid memory leaks every time I click on the gui
	// buttons
	void cleanForNewScene(void);
public:
	Scene();
	~Scene();
	
	void dbgDraw(void);
	void testRot(void);
	void testRaster(void);
	void testCameraLerp(void);
	void testCameraControl(void);
	void testCameraVis(void);
	void a2Demo(void);
	void a3Demo(void);
	void testTexture(void);
	void testBilTexLookup(void);
	void testSprites(void);
	// renders current FB as 3D point cloud in
	// an aux framebuffer.
	void renderFBAs3DPointCloud(void);
	void testShadowMap(void);
	void testTexProj(void);
	void testA4Demo(void);
	void testA4DemoExtra(void);
	void testCubeMapFaces(void);
	void testCubeMapReflection(void);
	void testCubeMapRefraction(void);
	void a5Demo(void);

	void saveCamera(void) const;
	void saveThisFramebuffer(void) const;

	void regFuncForKbRedraw(Scenes newScene);
	void currentSceneRedraw(void);
	void setDrawMode(int mode);
	void setMouseDelta(int mouseDeltaX, int mouseDeltaY);

	PPC* getCamera(void) { return ppc; }

	static const float K_HFOV; // field of view
	static const int K_W; // window height and width
	static const int K_H;
};

extern Scene *scene;

