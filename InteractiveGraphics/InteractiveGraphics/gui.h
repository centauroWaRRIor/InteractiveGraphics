// generated by Fast Light User Interface Designer (fluid) version 1.0108

#ifndef gui_h
#define gui_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>

class GUI {
public:
  GUI();
  Fl_Double_Window *uiw;
  Fl_Button *dbgButton;
private:
  void cb_dbgButton_i(Fl_Button*, void*);
  static void cb_dbgButton(Fl_Button*, void*);
  static Fl_Menu_Item menu_Main[];
public:
  static Fl_Menu_Item *A1TestRot;
private:
  void cb_A1TestRot_i(Fl_Menu_*, void*);
  static void cb_A1TestRot(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A1TestRaster;
private:
  void cb_A1TestRaster_i(Fl_Menu_*, void*);
  static void cb_A1TestRaster(Fl_Menu_*, void*);
public:
  Fl_Group *CameraFBSaveGroup;
  Fl_Button *SaveCameraButton;
  Fl_Button *SaveFBButotn;
  Fl_Group *RenderingOptionsGroup;
  Fl_Round_Button *WireframeRadialButton;
private:
  void cb_WireframeRadialButton_i(Fl_Round_Button*, void*);
  static void cb_WireframeRadialButton(Fl_Round_Button*, void*);
public:
  Fl_Round_Button *FlatRadialButton;
private:
  void cb_FlatRadialButton_i(Fl_Round_Button*, void*);
  static void cb_FlatRadialButton(Fl_Round_Button*, void*);
public:
  Fl_Round_Button *ScreenSpaceLerpRadialButton;
private:
  void cb_ScreenSpaceLerpRadialButton_i(Fl_Round_Button*, void*);
  static void cb_ScreenSpaceLerpRadialButton(Fl_Round_Button*, void*);
public:
  void show();
  void DBG_cb();
  void TestRot_cb();
  void TestRaster_cb();
  void DrawMode_cb(int mode);
};
#endif
