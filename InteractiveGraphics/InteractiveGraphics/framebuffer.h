#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>
#include <string>
using std::string;

// framebuffer + window class

class FrameBuffer : public Fl_Gl_Window {
public:
  unsigned int *pix; // SW color buffer
  int w, h; // image resolution
  FrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h); // constructor, top left coords and resolution
  ~FrameBuffer();

  // function that is always called back by system and never called directly by programmer
  // programmer triggers framebuffer update by calling FrameBuffer::redraw(), which makes
  //            system call draw
  void draw();

  // function called back when event occurs (mouse, keyboard, etc)
  int handle(int event);
  void KeyboardHandle();

  // clear to background color
  void Set(unsigned int color);
  // set one pixel function, check for frame boundaries
  void SetSafe(int u, int c, unsigned int color);
  // set one pixel function
  void Set(int u, int c, unsigned int color);
  // set to checkboard
  void SetCheckerboard(int checkerSize, unsigned int color0, 
    unsigned int color1);
  // draw circle
  void DrawCircle(float cuf, float cvf, float radius, unsigned int color);
  // draw axis aligned rectangle
  void DrawRectangle(float llu, float llv, float width, float height, unsigned int color);
  // draw 2D triangle
  void DrawTriangle(float *uCoords, float *vCoords, unsigned int color);

  // save as png image
  void SaveAsPng(string fname);
  // load from png image
  void LoadFromPng(string fname);
};


