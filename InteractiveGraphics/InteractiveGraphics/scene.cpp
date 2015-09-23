#include "scene.h"
#include <float.h>

#include <iostream>
#include <fstream>

using namespace std;

Scene *scene;

Scene::Scene() {

  // create user interface
  gui = new GUI();
  gui->show();

  // create SW framebuffer
  int u0 = 20;
  int v0 = 50;
  int sci = 2;
  int w = sci*240;
  int h = sci*180;
  fb = new FrameBuffer(u0, v0, w, h);
  fb->label("SW Framebuffer");
  fb->show();
  
  // position UI window
  gui->uiw->position(fb->w+u0 + 2*20, v0);

}



// function linked to the DBG GUI button for testing new features
void Scene::DBG() {

  cerr << "INFO: pressed DBG button" << endl;

}

