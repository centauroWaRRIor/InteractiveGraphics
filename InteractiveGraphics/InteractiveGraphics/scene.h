#pragma once

#include <iostream>
using namespace std;
#include "gui.h"
#include "framebuffer.h"
#include "tmesh.h"

class Scene {
private:
	FrameBuffer *fb; // SW framebuffer
	GUI * gui; // graphical user interface
	PPC *ppc; // camera used to render the scene from views chosen by user
	TMesh **tms; // an array of pointers to TMesh objects
	int tmsN; // how many TMeshes there are
public:
  Scene();
  ~Scene();
  void dbgInit();
  void dbgDraw();
  void testRot();
  void testRaster();
  void testCameraLerp();
  void A2();
  PPC* getCamera(void) { return ppc; }
};

extern Scene *scene;

