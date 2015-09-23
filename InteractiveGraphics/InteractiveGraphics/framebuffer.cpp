#include "framebuffer.h"
#include <iostream>
#include "scene.h"
#include <math.h>

using namespace std;

// makes an OpenGL window that supports SW, HW rendering, that can be displayed on screen
//        and that receives UI events, i.e. keyboard, mouse, etc.
FrameBuffer::FrameBuffer(int u0, int v0, 
  int _w, int _h) : Fl_Gl_Window(u0, v0, _w, _h, 0) {

  w = _w;
  h = _h;
  pix = new unsigned int[w*h];

}

// rendering callback; see header file comment
void FrameBuffer::draw() {

	// SW window, just transfer computed pixels from pix to HW for display
	glDrawPixels(w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);

}


// function called automatically on event within window (callback)
int FrameBuffer::handle(int event)  {  

  switch(event) 
  {   
  case FL_KEYBOARD: {
    KeyboardHandle();
    return 0;
  }
  default:
    break;
  }
  return 0;

}


void FrameBuffer::KeyboardHandle() {

  int key = Fl::event_key();
  switch (key) {
    case FL_Left: {
      cerr << "pressed left arrow" << endl;
      break;
    }
    case 'a': {
      cerr << "pressed a" << endl;
      break;
    }
    default:
    cerr << "INFO: do not understand keypress" << endl;
  }
}
