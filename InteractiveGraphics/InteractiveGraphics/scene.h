#pragma once
#pragma comment(lib,"fltk.lib")
#pragma comment(lib,"fltkgl.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"libtiff.lib")
#include "gui.h"
#include "framebuffer.h"

#include <iostream>

using namespace std;

class Scene {
public:


  FrameBuffer *fb; // SW framebuffer
  
  GUI * gui; // graphical user interface

  Scene();
  void DBG();
  

};

extern Scene *scene;

