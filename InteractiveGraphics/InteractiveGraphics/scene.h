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
	SPRITETEST};
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
	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	int tmsN; // how many TMeshes there are
	TMesh **tms; // an array of pointers to TMesh objects
	Texture **texObjects; // and array of pointers to Texture objects

	Scenes currentScene; // used for keyboard callback to invokate the correct redraw
	DrawModes currentDrawMode; // controls how to draw current scene
	// helps initializnig the different demo functions

	bool isA2Init, isDGBInit, isTestCamControlsInit,
		isTextureInit, isA3Init, isSpriteTestInit, 
		isTestBilTexLookupInit; 
	
	static const float hfov; // field of view
	static const int u0, v0; // initial window coordinates
	static const int w, h; // window height and width

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
		bool isAABBDrawn);
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

	void saveCamera(void) const;
	void saveThisFramebuffer(void) const;
	void regFuncForKbRedraw(Scenes newScene);
	void currentSceneRedraw(void);
	void setDrawMode(int mode);
	PPC* getCamera(void) { return ppc; }
};

extern Scene *scene;

