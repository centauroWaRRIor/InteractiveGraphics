#include "framebuffer.h"

// makes an OpenGL window that supports SW, HW rendering, that can be displayed on screen
//        and that receives UI events, i.e. keyboard, mouse, etc.
FrameBuffer::FrameBuffer(
	int u0, int v0,
	unsigned int _w, unsigned int _h) : 
	Fl_Gl_Window(u0, v0, _w, _h, 0),
	w(_w),
	h(_h) 
{
}

FrameBuffer::~FrameBuffer()
{
}

// function called automatically on event within window (callback)
int FrameBuffer::handle(int event) {

	// from users guide:
	// You must return non - zero if your handle() method uses the event.
	// If you return zero, the parent widget will try sending the event to another widget.
	switch (event)
	{
	case FL_KEYBOARD:
		keyboardHandle();
		return 1;
		// In order to receive FL_DRAG events, the widget must return non-zero when handling FL_PUSH.
	case FL_PUSH:
		if (Fl::event_button1()) {
			mouseLeftClickDragHandle(FL_PUSH);
			return 1;
		}
		else if (Fl::event_button3()) {
			mouseRightClickDragHandle(FL_PUSH);
			return 1;
		}
		return 0;
		// In order to receive the FL_RELEASE event, the widget must return non - zero when handling FL_PUSH.
	case FL_RELEASE:
		if (Fl::event_button1() || Fl::event_button3()) {
			return 1;
		}
		return 0;
	case FL_DRAG:
		if (Fl::event_button1()) {
			mouseLeftClickDragHandle(FL_DRAG);
			return 1;
		}
		else if (Fl::event_button3()) {
			mouseRightClickDragHandle(FL_DRAG);
			return 1;
		}
		return 0;
	default:
		return 0;
	}
}
