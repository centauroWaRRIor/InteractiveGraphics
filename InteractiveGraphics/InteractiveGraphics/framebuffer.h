#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/glut.h>

// framebuffer + window class

class FrameBuffer : public Fl_Gl_Window {
protected:
	int w, h; // image resolution
public:
	FrameBuffer(int u0, int v0, unsigned int _w, unsigned int _h); // constructor, top left coords and resolution
	virtual ~FrameBuffer();

	// because this function does not override the pure virtual draw function
	// from base class, this class is also still abstract. No need to redefine it
	// virtual void draw() = 0; 

	// simple getters
	int getWidth(void) const { return w; }
	int getHeight(void) const { return h; }

	// function called back when event occurs (mouse, keyboard, etc)
	virtual int handle(int event) override;
	virtual void keyboardHandle(void) = 0;
	virtual void mouseLeftClickDragHandle(int event) = 0;
	virtual void mouseRightClickDragHandle(int event) = 0;
};


