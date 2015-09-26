#pragma once

#include <iostream>
using namespace std;
#include "gui.h"
#include "framebuffer.h"
#include "tmesh.h"

enum class Scenes { DBG, A1, A2 };
enum class DrawModes { WIREFRAME, FLAT, SCREENSCAPELERP };

class Scene {
private:
	FrameBuffer *fb; // SW framebuffer
	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	TMesh **tms; // an array of pointers to TMesh objects
	int tmsN; // how many TMeshes there are
	Scenes currentScene; // used for keyboard callback to invokate the correct redraw
	DrawModes currentDrawMode; // controls how to draw current scene
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
	void regFuncForKbRedraw(Scenes newScene) {
		currentScene = newScene;
	}
	void currentSceneRedraw(void);
	void setDrawMode(int mode);
	PPC* getCamera(void) { return ppc; }
};

extern Scene *scene;

