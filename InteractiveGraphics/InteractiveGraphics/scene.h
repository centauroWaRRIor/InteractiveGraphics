#pragma once

#include <string>
using std::string;
#include "gui.h"
#include "framebuffer.h"
#include "tmesh.h"

enum class Scenes { DBG, A1, A2 };
enum class DrawModes { WIREFRAME, FLAT, SCREENSCAPELERP, DOTS };

class Scene {
private:
	FrameBuffer *fb; // SW framebuffer
	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	TMesh **tms; // an array of pointers to TMesh objects
	int tmsN; // how many TMeshes there are
	Scenes currentScene; // used for keyboard callback to invokate the correct redraw
	DrawModes currentDrawMode; // controls how to draw current scene
	
	// these two cameras are used as interpolation helpers
	PPC *ppcLerp0;
	PPC *ppcLerp1;

	// utlitliy function to retrieve time and date as a string
	string retrieveTimeDate(void) const;
public:
	Scene();
	~Scene();
	
	void dbgInit(void);
	void dbgDraw(void);
	void testRot(void);
	void testRaster(void);
	void testCameraLerp(void);
	void a2Init(void);
	void a2Draw(void);
	void saveCamera(void) const;
	void saveThisFramebuffer(void) const;
	void regFuncForKbRedraw(Scenes newScene) {
		currentScene = newScene;
	}
	void currentSceneRedraw(void);
	void setDrawMode(int mode);
	PPC* getCamera(void) { return ppc; }
};

extern Scene *scene;

