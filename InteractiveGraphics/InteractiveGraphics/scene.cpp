#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <float.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

Scene *scene;

Scene::Scene():
	fb(nullptr), 
    gui(nullptr),
    ppc(nullptr),
	tms(nullptr)
{

  // create user interface
  gui = new GUI();
  gui->show();

  // create SW framebuffer
  int u0 = 20;
  int v0 = 50;
  int sci = 2;
  int w = sci * 240; // 1280
  int h = sci * 180; // 720
  fb = new FrameBuffer(u0, v0, w, h);
  fb->label("SW Framebuffer");

  // create camera
  float hfov = 55.0f;
  ppc = new PPC(hfov, w, h);

  // allocate pointer of TMesh objects
  tmsN = 5;
  tms = new TMesh*[tmsN];
  for (int i = 0; i < 5; i++) {
	  tms[i] = nullptr;
  }

  fb->show();
  
  // position UI window
  gui->uiw->position(fb->getWidth()+u0 + 2*20, v0);

}

Scene::~Scene()
{
	delete fb;
	delete gui;
	delete ppc;
	// delete array of pointers
	for (int i = 0; i<tmsN; i++)
		delete[] tms[i];
	delete[] tms;
}

// function linked to the DBG GUI button for testing new features
void Scene::dbgDraw() {

	static bool doOnce = false;
	if (!doOnce) {
		dbgInit();
		doOnce = true;
	}
	fb->set(0xFFFFFFFF);
	fb->clearZB(0.0f);
	tms[0]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
	//tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
	//tms[0]->drawFilledFlat(*fb, *ppc, 0xFF0000FF);
	//tms[0]->drawWireframe(*fb, *ppc);
	tms[0]->drawVertexDots(*fb, *ppc, 3.0f);
	fb->redraw();
	return;
}

void Scene::dbgInit() {

	tms[0] = new TMesh();
	// test tetrahedron
	//tms[0]->createTetrahedronTestMesh();

	// test teapot
	ppc->moveForward(-200.0f);
	//ppc->moveForward(-1.0f);
	//ppc->moveUp(0.0f);// = V3(0.0f, 20.0f, 200.0f);
	tms[0]->loadBin("geometry/teapot1K.bin");
	
	//tms[0]->translate(V3(10.0f, -10.0f, 0.0f));
	//tms[0]->scale(1.0);

	//// test fitToAABB
	//AABB testAABB(tms[0]->getAABB());
	//testAABB.translate(V3(-40.0f, 0.0f, 0.0f));
	//testAABB.scale(2.0f);
	//tms[0]->setToFitAABB(testAABB);

	// test camera positioning functionality (works fine)
	//ppc->positionRelativeToPoint(
	//	verts[0], 
	//	V3(0.0f, -1.0f, 0.0f), 
	//	V3(0.0f, 0.0f, -1.0f), 
	//	100.0f);

	// test saving file (works fine)
	//ppc->saveCameraToFile("cameraSaveTest2.txt");
	//ppc->loadCameraFromFile("cameraSaveTest1.txt");
}

void Scene::testRot() {

	fb->set(0xFFFFFFFF);
	unsigned int pointColor = 0xFF0000FF;
	unsigned int originColor = 0xFFAA0000;
	unsigned int movingPointColor1 = 0xFF00FF80;
	unsigned int movingPointColor2 = 0xFFFF8000;
	unsigned int breadCrumbColor1 = 0xFF00CC66;
	unsigned int breadCrumbColor2 = 0xFFCC6600;
	float pointRadius = 5.0f;

	V3 point, axisDirection;
	V3 originalPoint(fb->getWidth() / 2.0f + 300.0f, fb->getHeight() / 2.0f, 0.0f);
	V3 axisOrigin(fb->getWidth() / 2.0f, fb->getHeight() / 2.0f, 0.0f);
	V3 originalAxisDirection(0.0f, 0.0f, 1.0f); // rotating about this axis creates a circle
	vector<V3> breadcrumbsCircle;
	vector<V3> breadcrumbsEllipse;

	// rotate z-axis 65 degrees around x-axis so that we get a nice ellipse
	M33 rotMatrix;
	rotMatrix.setRotationAboutX(65.0f);

	string pngFilename;
	
	for (int stepsi = 0; stepsi <= 360; stepsi++) {
		// reset point;
		point = originalPoint;
		// reset axis direction
		axisDirection = originalAxisDirection;
		// clear screen
		fb->set(0xFFFFFFFF);
		// draw origin point
		fb->draw2DCircle(axisOrigin[0], axisOrigin[1], pointRadius, originColor);
		// draw original point
		fb->draw2DCircle(point[0], point[1], pointRadius, pointColor);

		// draw rotating point about z-axis (makes a circle)
		point.rotateThisPointAboutAxis(axisOrigin, axisDirection, (float)stepsi);
		fb->draw2DCircle(point[0], point[1], pointRadius, movingPointColor1);
		// store new breadcrumb
		if (stepsi % 5 == 0) {
			breadcrumbsCircle.push_back(point);
		}

		// draw rotating point about rotated z-axis (makes an ellipse)
		// reset point;
		point = originalPoint;
		axisDirection = rotMatrix * axisDirection;
		point.rotateThisPointAboutAxis(axisOrigin, axisDirection, (float)stepsi);
		fb->draw2DCircle(point[0], point[1], pointRadius, movingPointColor2);
		// store new breadcrumb
		if (stepsi % 5 == 0) {
			breadcrumbsEllipse.push_back(point);
		}

		// draw circle breadcrums so far collected (gives nice hint of point trajectory)
		for (vector<V3>::iterator it = breadcrumbsCircle.begin(); it != breadcrumbsCircle.end(); ++it) {
			V3 &breadcrumb = *it;
			fb->draw2DCircle(breadcrumb[0], breadcrumb[1], pointRadius/2.0f, breadCrumbColor1);
		}

		// draw ellipse breadcrums so far collected (gives nice hint of point trajectory)
		for (vector<V3>::iterator it = breadcrumbsEllipse.begin(); it != breadcrumbsEllipse.end(); ++it) {
			V3 &breadcrumb = *it;
			fb->draw2DCircle(breadcrumb[0], breadcrumb[1], pointRadius / 2.0f, breadCrumbColor2);
		}
		// uncomment here to save video
		//pngFilename = string("pngs\\movieFrame");
		//pngFilename += std::to_string(stepsi) + ".png";
		//fb->SaveAsPng(pngFilename);
		fb->redraw();
		Fl::check();
	}
	return;
}

void Scene::testRaster() {

	fb->set(0xFFFFFFFF);
	unsigned int triangleColor = 0xFF0000FF;
	unsigned int circleColor = 0xFFAA0000;
	unsigned int rectangleColor = 0xFF00FF80;

	float circleRadius = 150.0f;
	V3 circleCenter(0 - circleRadius*2.0f, fb->getHeight() / 2.0f, 0.0f);

	// move circle from left to right, into scene and out
	for (float steps = 0 - (circleRadius*2.0f);
			steps <= fb->getWidth() + 2*(circleRadius*2.0f);
			steps += 10.0f) {
		// clear screen
		fb->set(0xFFFFFFFF);
		// draw circle
		fb->draw2DCircle(	circleCenter[0] + steps,
						circleCenter[1], 
						circleRadius,
						circleColor);
		fb->redraw();
		Fl::check();
	}

	float rectangleWidth = 400.0f;
	float rectangleHeight = 200.0f;
	V3 lowerLeft(fb->getWidth() / 2.0f, 0.0f, 0.0f);

	// move rectangle from bottom to top, into scene and out
	for (float steps = 0;
		steps <= fb->getHeight() + rectangleHeight;
		steps += 5.0f) {
		// clear screen
		fb->set(0xFFFFFFFF);
		// draw rectangle
		fb->draw2DRectangle(	lowerLeft[0], 
							lowerLeft[1] + steps,
							rectangleWidth,
							rectangleHeight,
							rectangleColor);
		fb->redraw();
		Fl::check();
	}

	// move triangle from right to left, into scene and out
	float triangleWidth = 400.0f;
	float triangleHeight = 200.0f;
	float xCoords[3] = { (float) fb->getWidth(),
						fb->getWidth() + (triangleWidth/2.0f),
						fb->getWidth() + triangleWidth };

	float yCoords[3] = { fb->getHeight() / 2.0f,
						(fb->getHeight() / 2.0f) - triangleHeight,
						(fb->getHeight() / 2.0f) - (triangleHeight/3.0f) };

	for (float steps = 0;
		steps <= fb->getWidth() + triangleWidth;
		steps += 0.5f)
	{
		// clear screen
		fb->set(0xFFFFFFFF);
		// draw triangle
		xCoords[0] = xCoords[0] - steps;
		xCoords[1] = xCoords[1] - steps;
		xCoords[2] = xCoords[2] - steps;

		fb->draw2DFlatTriangle(xCoords, yCoords, triangleColor);
		fb->redraw();
		Fl::check();
	}
	return;
}

void Scene::testCameraLerp(void)
{
	static bool doOnce = false;
	static PPC ppc0("cameraSaveTest1.txt");
	static PPC ppc1("cameraSaveTest2.txt");
	static int i = 0;
	static int n = 100;
	if (!doOnce) {
		dbgInit();
		doOnce = true;
	}
	 for (i = 0; i < n; i++) {

		fb->set(0xFFFFFFFF);
		ppc->setByInterpolation(ppc0, ppc1, i, n);
		//fb->drawWireFrame(tms[0], ppc); // replace with tmesh->drawWireFrame
		fb->redraw();
		Fl::check();
		Fl::wait(0.09);	
	}
	return;
}

void Scene::a2Init()
{
	tms[0] = new TMesh("geometry/teapot1K.bin");
	tms[1] = new TMesh("geometry/happy4.bin");
	// test tetrahedron
	//tms[0]->createTetrahedronTestMesh();

	AABB teapotAABB = tms[0]->getAABB();
	//teapotAABB.translate(V3(20.0f, 0.0f, 0.0f));
	tms[1]->setToFitAABB(teapotAABB);

	ppc->moveForward(-200.0f);
}

void Scene::a2Draw()
{
	static bool doOnce = false;
	if (!doOnce) {
		a2Init();
		doOnce = true;
	}
	
	//// rotates the mesh
	//int stepsN = 361;
	//float theta = 1.0f;
	//V3 center = tms[0]->getCenter();
	//V3 aDir(1.0f, 1.0f, 0.0f);
	//aDir.normalize();
	//for (int si = 0; si < stepsN; si++) {
	//	fb->set(0xFFFFFFFF);
	//	fb->clearZB(0.0f);
	//	//tms[0]->drawWireframe(*fb, *ppc);
	//	//tms[0]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
	//	//tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
	//	//tms[0]->drawFilledFlat(*fb, *ppc, 0xFF0000FF);
	//	tms[0]->drawVertexDots(*fb, *ppc, 3.0f);
	//	fb->redraw();
	//	Fl::check();
	//	tms[0]->rotateAboutAxis(center, aDir, theta);
	//}

	fb->set(0xFFFFFFFF);
	fb->clearZB(0.0f);

	// draws the meshes
	for (int i = 0; i < 2; i++) 
	{
		//tms[0]->drawWireframe(*fb, *ppc);
		//tms[0]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
		//tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
		//tms[0]->drawFilledFlat(*fb, *ppc, 0xFF0000FF);
		tms[i]->drawVertexDots(*fb, *ppc, 3.0f);
		tms[i]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
	}
	fb->redraw();

	return;
}

void Scene::currentSceneRedraw(void)
{
	switch (currentScene) {
	case Scenes::DBG:
		dbgDraw();
		break;
	case Scenes::A1:
		testRot();
		break;
	case Scenes::A2:
		a2Draw();
		break;
	default:
		dbgDraw();
		break; // this statement is optional for default
	}
}
