#include "scene.h"
#include "v3.h"
#include "m33.h"
#include <float.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

Scene *scene;

const float Scene::hfov = 55.0f;
const int Scene::u0 = 20;
const int Scene::v0 = 50;
const int Scene::w = 1280;
const int Scene::h = 720;

string Scene::retrieveTimeDate(void) const
{
	stringstream returnString;
	time_t t = time(0);   // get time now
	struct tm now;
	localtime_s(&now, &t);
	returnString << (now.tm_year + 1900) << '-'
				<< (now.tm_mon + 1) << '-'
				<< now.tm_mday << '-'
				<< now.tm_hour << '-'
				<< now.tm_min << '-'
				<< now.tm_sec;
	return returnString.str();
}

void Scene::cleanForScene(Scenes currentScene)
{
	if (currentScene == Scenes::A2) {
		for (int i = 0; i < tmsN; i++) {
			if (tms[i]) {
				delete tms[i];
				tms[i] = nullptr;
			}
		}
		if (ppcLerp0)
			delete ppcLerp0;
		if (ppcLerp1)
			delete ppcLerp1;
		ppcLerp0 = nullptr;
		ppcLerp1 = nullptr;
		// reset camera
		delete ppc;
		ppc = nullptr;
		ppc = new PPC(hfov, w, h);
		// reset init flags
		isDGBInit = false;
	}
	else if (currentScene == Scenes::DBG) {
		if (tms[0]) {
			delete tms[0];
			tms[0] = nullptr;
		}
		if (ppcLerp0)
			delete ppcLerp0;
		if (ppcLerp1)
			delete ppcLerp1;
		ppcLerp0 = nullptr;
		ppcLerp1 = nullptr;
		// reset camera
		delete ppc;
		ppc = nullptr;
		ppc = new PPC(hfov, w, h);
		// reset init flags
		isA2Init = false;
	}
	else if (currentScene == Scenes::CAMLERP) {
		if (tms[0]) {
			delete tms[0];
			tms[0] = nullptr;
		}
		if (ppcLerp0)
			delete ppcLerp0;
		if (ppcLerp1)
			delete ppcLerp1;
		ppcLerp0 = nullptr;
		ppcLerp1 = nullptr;
		// reset camera
		delete ppc;
		ppc = nullptr;
		ppc = new PPC(hfov, w, h);
		// reset init flags
		isA2Init = false;
		isDGBInit = false;
	}
}

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
  fb = new FrameBuffer(u0, v0, w, h);
  fb->label("SW Framebuffer");

  // create camera
  ppc = new PPC(hfov, w, h);
  ppcLerp0 = nullptr;
  ppcLerp1 = nullptr;

  // allocate pointer of TMesh objects
  tmsN = 6;
  tms = new TMesh*[tmsN];
  for (int i = 0; i < tmsN; i++) {
	  tms[i] = nullptr;
  }

  fb->show();
  
  // position UI window
  gui->uiw->position(fb->getWidth()+u0 + 2*20, v0);

  // set default draw mode wireframe
  currentDrawMode = DrawModes::WIREFRAME;

  isA2Init = false;
  isDGBInit = false;
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
	if (ppcLerp0)
		delete ppcLerp0;
	if (ppcLerp1)
		delete ppcLerp1;
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
	if (currentDrawMode == DrawModes::FLAT)
		tms[0]->drawFilledFlat(*fb, *ppc, 0xFFFF0000);
	else if (currentDrawMode == DrawModes::SCREENSCAPELERP)
		tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
	else if (currentDrawMode == DrawModes::WIREFRAME)
		tms[0]->drawWireframe(*fb, *ppc);
	else if (currentDrawMode == DrawModes::DOTS)
		tms[0]->drawVertexDots(*fb, *ppc, 2.0f);
	tms[0]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
	fb->redraw();
	return;
}

void Scene::dbgInit() {

	cleanForScene(Scenes::DBG);
	tms[0] = new TMesh();

	// test teapot
	ppc->moveForward(-200.0f);
	
	tms[0]->loadBin("geometry/teapot1K.bin");
	
	tms[0]->translate(V3(10.0f, -10.0f, 0.0f));
	tms[0]->scale(1.0);

	//// test fitToAABB (works fine)
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

	// test saving/loading file (works fine)
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

#ifdef _MAKE_VIDEO_
	string pngFilename;
#endif	
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
#ifdef _MAKE_VIDEO_
		pngFilename = string("pngs\\movieFrame");
		pngFilename += std::to_string(stepsi) + ".png";
		fb->SaveAsPng(pngFilename);
#endif
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
	// test simple cam lerp with simple tetrahedron
	cleanForScene(Scenes::CAMLERP);
	tms[0] = new TMesh();
	ppcLerp0 = new PPC("camera_saves\\cameraSaveTest1.txt");
	ppcLerp1 = new PPC("camera_saves\\cameraSaveTest2.txt");
	static int i = 0;
	static int n = 100;
	tms[0]->createTetrahedronTestMesh();

	 for (i = 0; i < n; i++) {

		// clear screen
		fb->set(0xFFFFFFFF);
		fb->clearZB(0.0f);

		ppc->setByInterpolation(*ppcLerp0, *ppcLerp1, i, n);
		if (currentDrawMode == DrawModes::FLAT)
			tms[0]->drawFilledFlat(*fb, *ppc, 0xFFFF0000);
		else if (currentDrawMode == DrawModes::SCREENSCAPELERP)
			tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
		else if (currentDrawMode == DrawModes::WIREFRAME)
			tms[0]->drawWireframe(*fb, *ppc);
		else if (currentDrawMode == DrawModes::DOTS)
			tms[0]->drawVertexDots(*fb, *ppc, 2.0f);
		tms[0]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
		fb->redraw();
		Fl::check();
//		Fl::wait(0.09);	
	}
	return;
}

void Scene::a2Init()
{
	cleanForScene(Scenes::A2);

	tms[0] = new TMesh("geometry/teapot1K.bin");
	tms[1] = new TMesh("geometry/teapot1K.bin");
	tms[2] = new TMesh("geometry/teapot1K.bin");
	tms[3] = new TMesh("geometry/teapot1K.bin");
	tms[4] = new TMesh("geometry/happy4.bin");
	tms[5] = new TMesh("geometry/terrain.bin");

	ppcLerp0 = new PPC("camera_saves\\2015-9-26-21-42-57-camera.txt");
	ppcLerp1 = new PPC("camera_saves\\2015-9-26-22-42-55-camera.txt");

	// scale happy mesh to be same scale as teapots
	AABB teapotAABB = tms[0]->getAABB();
	tms[4]->setToFitAABB(teapotAABB);

	// scale terrain mesh to multiple of teapot reference
	tms[5]->setToFitAABB(teapotAABB);
	tms[5]->rotateAboutAxis(tms[0]->getCenter(), V3(1.0f, 0.0f, 0.0f), -90.0f);
	tms[5]->rotateAboutAxis(tms[0]->getCenter(), V3(0.0f, 1.0f, 0.0f), 180.0f);
	tms[5]->scale(4.0);
	tms[5]->translate(V3(50.0f, -200.0f, 120.0f));

	// place teapots at a cross formation around happy
	tms[0]->translate(V3(200.0f, 0.0f, 0.0f));
	tms[1]->translate(V3(-200.0f, 0.0f, 0.0f));
	tms[2]->translate(V3(-200.0f, 90.0f, 0.0f));
	tms[3]->translate(V3(200.0f, 90.0f, 0.0f));
	
	// place initial position for camera
	ppc->moveForward(-600.0f);
}

void Scene::a2Draw()
{
	if (!isA2Init) {
		a2Init();
		isA2Init = true;
	}

	// for camera interpolations
	static int cameraLerpSteps = 120;
	static int cameraLerpStep = 0;
	
	// rotates the mesh
	int stepsN = 300; // 30 fps * 10 = 300
	float theta = 1.0f;
	V3 center = tms[4]->getCenter();
	V3 aDir(0.0f, 1.0f, 0.0f);
	aDir.normalize();
#ifdef _MAKE_VIDEO_
	string pngFilename;
#endif
	for (int si = 0; si < stepsN; si++) {

		// camera lerping
		if (si >= 150 && cameraLerpStep < cameraLerpSteps) {
			ppc->setByInterpolation(*ppcLerp0, *ppcLerp1, cameraLerpStep, cameraLerpSteps);
		}

		// clear screen
		fb->set(0xFFFFFFFF);
		fb->clearZB(0.0f);

		// draws background terrain, this doesn't rotate
		if(currentDrawMode == DrawModes::FLAT)
			tms[5]->drawFilledFlat(*fb, *ppc, 0xFFFF0000);
		else if(currentDrawMode == DrawModes::SCREENSCAPELERP)			
			tms[5]->drawFilledFlatBarycentric(*fb, *ppc);
		else if (currentDrawMode == DrawModes::WIREFRAME)
			tms[5]->drawWireframe(*fb, *ppc);
		else if (currentDrawMode == DrawModes::DOTS)
			tms[5]->drawVertexDots(*fb, *ppc, 2.0f);

		// draws the meshes that rotate
		for (int i = 0; i < 5; i++)
		{
			if (currentDrawMode == DrawModes::FLAT)
				tms[i]->drawFilledFlat(*fb, *ppc, 0xFFFF0000);
			else if (currentDrawMode == DrawModes::SCREENSCAPELERP)
				tms[i]->drawFilledFlatBarycentric(*fb, *ppc);
			else if (currentDrawMode == DrawModes::WIREFRAME)
				tms[i]->drawWireframe(*fb, *ppc);
			else if (currentDrawMode == DrawModes::DOTS)
				tms[i]->drawVertexDots(*fb, *ppc, 2.0f);
			tms[i]->drawAABB(*fb, *ppc, 0xFF00FF00, 0xFF000000);
		}

		fb->redraw();
		Fl::check();
		// rotate teapots
		tms[0]->rotateAboutAxis(center, aDir, theta);
		tms[1]->rotateAboutAxis(center, aDir, theta);
		tms[2]->rotateAboutAxis(center, aDir, -theta);
		tms[3]->rotateAboutAxis(center, aDir, -theta);
		// rotate happy
		tms[4]->rotateAboutAxis(center, aDir, theta);
		// increase camera interpolation step
		if (si >= 150 && cameraLerpStep < cameraLerpSteps)
			cameraLerpStep++;
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
#endif
	}
	return;
}

void Scene::saveCamera(void) const
{
	string filename = retrieveTimeDate();
	filename.append("-camera.txt");
	cout << "Wrote " << filename << endl;
	ppc->saveCameraToFile(filename);
}

void Scene::saveThisFramebuffer(void) const
{
	string filename = retrieveTimeDate();
	filename.append("-framebuffer.png");
	cout << "Wrote " << filename << endl;
	fb->saveAsPng(filename);
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
	case Scenes::CAMLERP:
		testCameraLerp();
		break;
	default:
		dbgDraw();
		break; // this statement is optional for default
	}
}

void Scene::setDrawMode(int mode)
{
	switch (mode) {

	case 1: 
		currentDrawMode = DrawModes::WIREFRAME;
		break;
	case 2:
		currentDrawMode = DrawModes::FLAT;
		break;
	case 3:
		currentDrawMode = DrawModes::SCREENSCAPELERP;
		break;
	case 4:
		currentDrawMode = DrawModes::DOTS;
		break;
	default:
		currentDrawMode = DrawModes::WIREFRAME;
		break; // optional statement for default
	}
	if (currentScene == Scenes::DBG ||
		currentScene == Scenes::CAMLERP) {
		currentSceneRedraw();
	}
}
