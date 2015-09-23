#pragma once

#include "gui.h"
#include "framebuffer.h"

#include <iostream>

using namespace std;

class Scene {
public:

  FrameBuffer *fb; // SW framebuffer
  GUI * gui; // graphical user interface

  Scene();
  ~Scene();
  void DBG();
  void TestRot();
  void TestRaster();
};

extern Scene *scene;

