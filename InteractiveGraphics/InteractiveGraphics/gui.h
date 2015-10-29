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
  static Fl_Menu_Item *A1SubMenu;
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
  static Fl_Menu_Item *A2SubMenu;
  static Fl_Menu_Item *A2TestCamLerp;
private:
  void cb_A2TestCamLerp_i(Fl_Menu_*, void*);
  static void cb_A2TestCamLerp(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestCamControls;
private:
  void cb_TestCamControls_i(Fl_Menu_*, void*);
  static void cb_TestCamControls(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A2Demo;
private:
  void cb_A2Demo_i(Fl_Menu_*, void*);
  static void cb_A2Demo(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestCamVis;
private:
  void cb_TestCamVis_i(Fl_Menu_*, void*);
  static void cb_TestCamVis(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A3SubMenu;
  static Fl_Menu_Item *TestTexture;
private:
  void cb_TestTexture_i(Fl_Menu_*, void*);
  static void cb_TestTexture(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestSprite;
private:
  void cb_TestSprite_i(Fl_Menu_*, void*);
  static void cb_TestSprite(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A3Demo;
private:
  void cb_A3Demo_i(Fl_Menu_*, void*);
  static void cb_A3Demo(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestBilTexLookup;
private:
  void cb_TestBilTexLookup_i(Fl_Menu_*, void*);
  static void cb_TestBilTexLookup(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A4SubMenu;
  static Fl_Menu_Item *TestFBAs3DCloud;
private:
  void cb_TestFBAs3DCloud_i(Fl_Menu_*, void*);
  static void cb_TestFBAs3DCloud(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestShadowMap;
private:
  void cb_TestShadowMap_i(Fl_Menu_*, void*);
  static void cb_TestShadowMap(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *TestTexProj;
private:
  void cb_TestTexProj_i(Fl_Menu_*, void*);
  static void cb_TestTexProj(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A4Demo;
private:
  void cb_A4Demo_i(Fl_Menu_*, void*);
  static void cb_A4Demo(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A4DemoExtra;
private:
  void cb_A4DemoExtra_i(Fl_Menu_*, void*);
  static void cb_A4DemoExtra(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *A5SubMenu;
  static Fl_Menu_Item *TestCubeMapFaces;
private:
  void cb_TestCubeMapFaces_i(Fl_Menu_*, void*);
  static void cb_TestCubeMapFaces(Fl_Menu_*, void*);
public:
  Fl_Group *CameraFBSaveGroup;
  Fl_Button *SaveCameraButton;
private:
  void cb_SaveCameraButton_i(Fl_Button*, void*);
  static void cb_SaveCameraButton(Fl_Button*, void*);
public:
  Fl_Button *SaveFBButotn;
private:
  void cb_SaveFBButotn_i(Fl_Button*, void*);
  static void cb_SaveFBButotn(Fl_Button*, void*);
public:
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
  Fl_Round_Button *DotsRadialButton;
private:
  void cb_DotsRadialButton_i(Fl_Round_Button*, void*);
  static void cb_DotsRadialButton(Fl_Round_Button*, void*);
public:
  Fl_Round_Button *ModelSpaceLerpRadialButton;
private:
  void cb_ModelSpaceLerpRadialButton_i(Fl_Round_Button*, void*);
  static void cb_ModelSpaceLerpRadialButton(Fl_Round_Button*, void*);
public:
  Fl_Round_Button *TextureRadialButton;
private:
  void cb_TextureRadialButton_i(Fl_Round_Button*, void*);
  static void cb_TextureRadialButton(Fl_Round_Button*, void*);
public:
  Fl_Button *A4Button;
private:
  void cb_A4Button_i(Fl_Button*, void*);
  static void cb_A4Button(Fl_Button*, void*);
public:
  void show();
  void DBG_cb();
  void TestRot_cb();
  void TestRaster_cb();
  void DrawMode_cb(int mode);
  void SaveCameraButton_cb();
  void SaveFB_cb();
  void A2Button_cb();
  void TestCamLerp_cb();
  void TestCamControls_cb();
  void TestCamVis_cb();
  void TestTexture_cb();
  void TestSprites_cb();
  void A3Demo_cb();
  void TestBilTexLookup_cb();
  void TestFBAs3DCloud_cb();
  void TestShadowMap_cb();
  void TestTexProj_cb();
  void A4Demo_cb();
  void A4DemoExtra_cb();
  void TestCubeMapFaces_cb();
};
#endif
