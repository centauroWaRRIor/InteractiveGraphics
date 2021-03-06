#pragma once
#include <string>
using std::string;
#include "gui.h"
// forward declaration here to prevent glew include
// sensibilities
class HWFrameBuffer;
class SWFrameBuffer;
class LightProjector;
class Texture;
class Light;
class TMesh;
class PPC;

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
	REFRACTEST,
	A6,
	A6_2};

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
	SWFrameBuffer *fb; // SW framebuffer
	SWFrameBuffer *fbAux; // currently used for rendering a FB as a 3D point cloud

	// hardware support is an additional feature, not a necessary component
	HWFrameBuffer *fixedHwFb; // fixed HW pipeline framebuffer
	HWFrameBuffer *progrHwFb; // programmable HW pipeline framebuffer
	HWFrameBuffer *reflectionshwFb; // programmable HW pipeline framebuffer demoing reflections

	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	Light *light; // light used to render models in lit mode
	LightProjector *lightProjector; // light used to project textures
	int tmsN; // how many TMeshes there are
	TMesh **tms; // an array of pointers to TMesh objects
	Texture **texObjects; // and array of pointers to Texture objects

	// use for camera dolly with the mouse in REFLECTEST and A6 Demos
	int mouseDeltaX, mouseDeltaY, mouseDeltaZ;

	Scenes currentScene; // used for keyboard callback to invokate the correct redraw
	DrawModes currentDrawMode; // controls how to draw current scene
	// helps initializnig the different demo functions

	bool isA2Init, isDGBInit, isTestCamControlsInit,
		isTextureInit, isA3Init, isSpriteTestInit, 
		isTestBilTexLookupInit, isShadowMapTestInit,
		isTexProjectTestInit, isA4DemoInit, isA4DemoExtraInit,
		isTestCMReflectInit, isTestCMRefractInit,
		isTestFixedPipelineInit, isTestProgrPipelineInit,
		isA6DemoInit, isA6Demo2Init;
	
	static const int u0, v0; // initial window coordinates

	// these two cameras are used as interpolation helpers
	PPC *ppcLerp0;
	PPC *ppcLerp1;

	// utlitliy function to retrieve time and date as a string
	string retrieveTimeDate(void) const;
	// utility function to centralize all triangle mesh drawing options
	void drawTMesh(
		TMesh &tMesh, 
		SWFrameBuffer &fB, 
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
	void testFixedPipelineHW(void);
	void testProgrPipelineHW(void);
	void a6Demo(void);
	void a6Demo2(void);

	void saveCamera(void) const;
	void saveThisFramebuffer(void) const;

	void regFuncForKbRedraw(Scenes newScene);
	void currentSceneRedraw(void);
	void setDrawMode(int mode);
	void setMouseDelta(int mouseDeltaX, int mouseDeltaY);
	void setMouseRoll(int mouseRoll);

	PPC* getCamera(void) { return ppc; }

	static const float K_HFOV; // field of view
	static const int K_W; // window height and width
	static const int K_H;
};

extern Scene *scene;

