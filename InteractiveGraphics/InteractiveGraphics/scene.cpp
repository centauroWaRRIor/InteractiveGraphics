#include "scene.h"
#include "v3.h"
#include "m33.h"
#include "hw_framebuffer.h"
#include "hw_progrpipeline.h"
#include "hw_fixedpipeline.h"
#include "hw_reflections.h"
#include "sw_framebuffer.h"
#include "tmesh.h"
#include "ppc.h"
#include "light.h"
#include "lightprojector.h"
#include "texture.h"
#include "cubemap.h"
#include <float.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cfloat>

using namespace std;

Scene *scene;

const int Scene::u0 = 20;
const int Scene::v0 = 50;
const float Scene::K_HFOV = 55.0f;
const int Scene::K_W = 1280;
const int Scene::K_H = 720;

Scene::Scene() :
	fb(nullptr),
	fbAux(nullptr),
	fixedHwFb(nullptr),
	progrHwFb(nullptr),
	reflectionshwFb(nullptr),
	gui(nullptr),
	ppc(nullptr),
	light(nullptr),
	lightProjector(nullptr),
	tms(nullptr),
	texObjects(nullptr),
	mouseDeltaX(0),
	mouseDeltaY(0),
	mouseDeltaZ(0)
{

	// create user interface
	gui = new GUI();
	gui->show();

	// create persistent SW framebuffer
	fb = new SWFrameBuffer(u0, v0, K_W, K_H);
	fb->label("SW Framebuffer");

	// create camera
	ppc = new PPC(K_HFOV, K_W, K_H);
	ppcLerp0 = nullptr;
	ppcLerp1 = nullptr;

	// create scene's light
	light = new Light();

	// allocate pointer of TMesh objects
	// and allocate pointer of Texture objects
	// tmsN is used as a bound for the maximum
	// number of Texture objects that can be at once
	tmsN = 6;
	tms = new TMesh*[tmsN];
	texObjects = new Texture*[tmsN];
	for (int i = 0; i < tmsN; i++) {
		tms[i] = nullptr;
		texObjects[i] = nullptr;
	}

	fb->show();

	// position UI window
	gui->uiw->position(fb->getWidth() + u0 + 2 * 20, v0);

	// set default draw mode wireframe
	currentDrawMode = DrawModes::WIREFRAME;

	this->isDGBInit = false;
	this->isTestCamControlsInit = false;
	this->isA2Init = false;
	this->isTestCamControlsInit = false;
	this->isTextureInit = false;
	this->isSpriteTestInit = false;
	this->isA3Init = false;
	this->isTestBilTexLookupInit = false;
	this->isShadowMapTestInit = false;
	this->isTexProjectTestInit = false;
	this->isTestCMReflectInit = false;
	this->isTestCMRefractInit = false;
	this->isTestFixedPipelineInit = false;
	this->isTestProgrPipelineInit = false;
	this->isA6DemoInit = false;
}

Scene::~Scene()
{
	delete fb;
	if (fbAux)
		delete fbAux;
	if (fixedHwFb)
		delete fixedHwFb;
	if (progrHwFb)
		delete progrHwFb;
	if (reflectionshwFb)
		delete reflectionshwFb;
	delete gui;
	delete ppc;
	delete light;
	// delete array of pointers
	for (int i = 0; i < tmsN; i++) {
		delete tms[i];
		delete texObjects[i];
	}
	if (lightProjector) {
		delete lightProjector;
	}
	delete[] tms;
	delete[] texObjects;
	if (ppcLerp0)
		delete ppcLerp0;
	if (ppcLerp1)
		delete ppcLerp1;
}

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

void Scene::drawTMesh(
	TMesh & tMesh, 
	SWFrameBuffer & frameBuffer, 
	const PPC & planarPinholeCamera,
	bool isAABBDrawn,
	bool isShadowsEnabled,
	bool isLightProjEnabled)
{
	if (currentDrawMode == DrawModes::DOTS)
		tMesh.drawVertexDots(frameBuffer, planarPinholeCamera, 2.0f);
	else if (currentDrawMode == DrawModes::WIREFRAME)
		tMesh.drawWireframe(frameBuffer, planarPinholeCamera);
	else if (currentDrawMode == DrawModes::FLAT)
		tMesh.drawFilledFlat(frameBuffer, planarPinholeCamera, 0xFFFF0000);
	else if (currentDrawMode == DrawModes::SCREENSCAPELERP)
		tMesh.drawFilledFlatBarycentric(frameBuffer, planarPinholeCamera);
	else if (currentDrawMode == DrawModes::MODELSPACELERP)
		tMesh.drawFilledFlatPerspCorrect(frameBuffer, planarPinholeCamera);
	else if (currentDrawMode == DrawModes::LIT)
		tMesh.drawLit(
			frameBuffer,
			planarPinholeCamera,
			*light,
			lightProjector,
			nullptr, // not using lit + texture mode here
			false,
			isShadowsEnabled,
			isLightProjEnabled);

	// drawing of AABB is overruled (not to be done) when drawing
	// in model space interpolation mode. Reason being that drawAABB
	// uses drawWireframe which in turn assumes depth test logic of 1/w near.
	// Model Space interpolation mode changes depth test near logic
	// to w near.
	if(isAABBDrawn && currentDrawMode != DrawModes::MODELSPACELERP)
		tMesh.drawAABB(frameBuffer, planarPinholeCamera, 0xFF00FF00, 0xFF000000);
}

void Scene::cleanForNewScene(void)
{
	// clean TMeshes and Texture objects
	for (int i = 0; i < tmsN; i++) {
		if (tms[i]) {
			delete tms[i];
			tms[i] = nullptr;
		}
		if (texObjects[i]) {
			delete texObjects[i];
			texObjects[i] = nullptr;
		}
	}
	
	// Reset interpolation cameras
	if (ppcLerp0) {
		delete ppcLerp0;
		ppcLerp0 = nullptr;

	}
	if (ppcLerp1) {
		delete ppcLerp1;
		ppcLerp1 = nullptr;
	}

	// reset main camera
	delete ppc;
	ppc = nullptr;
	ppc = new PPC(K_HFOV, K_W, K_H);

	// reset main light
	delete light;
	light = nullptr;
	light = new Light();

	// clean light projector
	if (lightProjector) {
		delete lightProjector;
		lightProjector = nullptr;
	}

	// destroy fbAux if present
	if (fbAux) {
		delete fbAux;
		fbAux = nullptr;
	}

	// destroy fixedHwFb if present
	if (fixedHwFb) {
		delete fixedHwFb;
		fixedHwFb = nullptr;
	}

	// destroy progrHwFb if present
	if (progrHwFb) {
		delete progrHwFb;
		progrHwFb = nullptr;
	}

	// destroy progrHwFb if present
	if (reflectionshwFb) {
		delete reflectionshwFb;
		reflectionshwFb = nullptr;
	}

	// reset init flags for the different scenes
	this->isDGBInit = false;
	this->isTestCamControlsInit = false;
	this->isA2Init = false;
	this->isTestCamControlsInit = false;
	this->isTextureInit = false;
	this->isSpriteTestInit = false;
	this->isA3Init = false;
	this->isTestBilTexLookupInit = false;
	this->isShadowMapTestInit = false;
	this->isTexProjectTestInit = false;
	this->isA4DemoInit = false;
	this->isA4DemoExtraInit = false;
	this->isTestCMReflectInit = false;
	this->isTestCMRefractInit = false;
	this->isTestFixedPipelineInit = false;
	this->isTestProgrPipelineInit = false;
	this->isA6DemoInit = false;
}

// function linked to the DBG GUI button for testing new features
void Scene::dbgDraw() {

	if (!isDGBInit) {

		cleanForNewScene();

		// create progr pipeline HW framebuffer
		reflectionshwFb = new HWReflections(u0, v0, K_W, K_H);
		reflectionshwFb->label("Programmable Pipeline HW Framebuffer for A6");

		unsigned int tmsN = 5;
		unsigned int n;
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();
		tms[4] = new TMesh();

		tms[0]->createQuadTMesh(false);
		tms[0]->translate(V3(-100.0f, 50.0f, 0.0f));
		tms[1]->createQuadTMesh(false);
		tms[1]->translate(V3(20.0f, 50.0f, 0.0f));
		tms[2]->createQuadTMesh(true);
		tms[2]->translate(V3(-100.0f, 0.0f, 0.0f));
		tms[3]->createQuadTMesh(true);
		tms[3]->translate(V3(20.0f, 0.0f, 0.0f));
		tms[4]->loadBin("geometry/teapot1K.bin");
		tms[4]->disableTexCoords(); // they don't look good
		for (n = 0; n < tmsN; n++) {
			reflectionshwFb->registerTMesh(tms[n]);
		}

		// load five different textures for demoing
		unsigned int texObjectsN = 6;
		texObjects[0] = new Texture("pngs\\White_brick_block_pxr128.png");
		texObjects[1] = new Texture("pngs\\Macbeth_color_checker_pxr128.png"); // test tiling
		texObjects[2] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");
		texObjects[3] = new Texture("pngs\\Brown_staggered_pxr128.png");
		texObjects[4] = new Texture("pngs\\Woven_flower_pxr128.png");
		texObjects[5] = new Texture("pngs\\American_walnut_pxr128.png");
		for (n = 0; n < texObjectsN; n++) {
			reflectionshwFb->registerTexture(texObjects[n]);
		}

		// assign tMesh to texture mappings
		reflectionshwFb->assignTMeshTexture(0, 0);
		reflectionshwFb->assignTMeshTexture(1, 1);
		reflectionshwFb->assignTMeshTexture(2, 2);
		reflectionshwFb->assignTMeshTexture(3, 3);
		reflectionshwFb->assignTMeshTexture(4, 3);

		V3 center = tms[4]->getCenter();
		ppc->moveForward(-200.0f);
		ppc->positionAndOrient(ppc->getEyePoint(), center, V3(0.0f, 1.0f, 0.0f));
		ppc->moveUp(10.0f);
		reflectionshwFb->registerPPC(ppc);

		reflectionshwFb->show();
		isDGBInit = true;		
	}
	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	drawTMesh(*tms[0], *fb, *ppc, true);
	
	fb->redraw();
	reflectionshwFb->redraw();
	return;
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
		fb->draw2DRectangle(lowerLeft[0], 
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
	V3 tProjVerts[3];
	tProjVerts[0] = V3((float)fb->getWidth(), fb->getHeight() / 2.0f);
	tProjVerts[1] = V3(fb->getWidth() + (triangleWidth / 2.0f), (fb->getHeight() / 2.0f) - triangleHeight);
	tProjVerts[2] = V3(fb->getWidth() + triangleWidth, (fb->getHeight() / 2.0f) - (triangleHeight / 3.0f));

	for (float steps = 0;
		steps <= fb->getWidth() + triangleWidth;
		steps += 0.5f)
	{
		// clear screen
		fb->set(0xFFFFFFFF);
		// draw triangle
		tProjVerts[0][0] -= steps;
		tProjVerts[1][0] -= steps;
		tProjVerts[2][0] -= steps;
		fb->draw2DFlatTriangle(tProjVerts, triangleColor);
		fb->redraw();
		Fl::check();
	}
	return;
}

void Scene::testCameraLerp(void)
{
	// test simple cam lerp with simple tetrahedron
	cleanForNewScene();
	tms[0] = new TMesh();
	ppcLerp0 = new PPC("camera_saves\\cameraSaveTest1.txt");
	ppcLerp1 = new PPC("camera_saves\\cameraSaveTest2.txt");
	static int i = 0;
	static int n = 100;
	tms[0]->createTetrahedronTestMesh();

	 for (i = 0; i < n; i++) {

		// clear screen
		fb->set(0xFFFFFFFF);
		// clear zBuffer
		if (currentDrawMode == DrawModes::MODELSPACELERP)
			fb->clearZB(FLT_MAX);
		else
			fb->clearZB(0.0f);

		ppc->setByInterpolation(*ppcLerp0, *ppcLerp1, i, n);
		drawTMesh(*tms[0], *fb, *ppc, true);
		fb->redraw();
		Fl::check();
//		Fl::wait(0.09);	
	}
	return;
}

void Scene::testCameraControl(void)
{
	if (!isTestCamControlsInit) {

		cleanForNewScene();
		tms[0] = new TMesh();

		// test teapot
		ppc->moveForward(-200.0f);

		// set up light
		light->setAmbientK(0.4f);
		light->setMatColor(V3(1.0f, 0.0f, 0.0f));
		light->setPosition(ppc->getEyePoint());

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

		isTestCamControlsInit = true;
	}
	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	drawTMesh(*tms[0], *fb, *ppc, true);
	fb->redraw();
	return;
}

void Scene::testCameraVis(void)
{
	PPC cam(60.0f, fb->getWidth(), fb->getHeight());
	cam.translate(V3(0.0f, 0.0f, -100.0f));
	for (int i = 0; i < 361; i++) {
		fb->clearZB(0.0f);
		fb->set(0xFFFFFFFF);
		cam.visualizeCamera(*ppc, *fb, 40.0f);
		fb->redraw();
		Fl::check();
		cam.pan(1.0f);
	}
	return;
}

void Scene::a2Demo()
{
	if (!isA2Init) {
		
		cleanForNewScene();

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
		// clear zBuffer
		if (currentDrawMode == DrawModes::MODELSPACELERP)
			fb->clearZB(FLT_MAX);
		else
			fb->clearZB(0.0f);

		// draws background terrain, this doesn't rotate
		drawTMesh(*tms[5], *fb, *ppc, false);

		// draws the meshes that rotate
		for (int i = 0; i < 5; i++)
			drawTMesh(*tms[i], *fb, *ppc, true);

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

void Scene::a3Demo(void)
{
	if (!this->isA3Init) {

		cleanForNewScene();
		// prepare 6 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();
		tms[4] = new TMesh();
		tms[5] = new TMesh();

		// look at quads full frontal
		ppc->moveForward(-180.0f);
		ppc->moveUp(100.0f);
		ppc->moveRight(20.0f);
		ppc->tilt(-25);
		// save initial camera settings into ppcLerp0
		ppcLerp0 = new PPC(*ppc);

		// enable tiling for all the quads
		tms[0]->createQuadTMesh(false);
		tms[1]->createQuadTMesh(true);
		tms[2]->createQuadTMesh(true);
		tms[3]->createQuadTMesh(false);
		tms[4]->createQuadTMesh(false);
		tms[5]->createQuadTMesh(true);

		// load five different textures for demoing
		texObjects[0] = new Texture("pngs\\White_brick_block_pxr128.png");
		texObjects[1] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");
		texObjects[2] = new Texture("pngs\\Brown_staggered_pxr128.png");
		texObjects[3] = new Texture("pngs\\Woven_flower_pxr128.png");
		texObjects[4] = new Texture("pngs\\MeAtPyramid.png");
		texObjects[5] = new Texture("pngs\\American_walnut_pxr128.png");

		// use 5 quads to cover five faces of a 3D cube
		// tms[0] is the front face
		// tms[1] is the left face
		tms[1]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), -90.0f);
		// push in place
		tms[1]->translate(V3(0.0f, 0.0f, -40.0f));
		// tms[2] is the right face
		tms[2]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), 90.0f);
		// push in place
		tms[2]->translate(V3(40.0f, 0.0f, 0.0f));
		// tms[3] is the back face
		tms[3]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), 180.0f);
		// push in place
		tms[3]->translate(V3(40.0f, 0.0f, -40.0f));
		// tms[4] is the top face
		tms[4]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), -90.0f);
		// push in place
		tms[4]->translate(V3(0.0f, 40.0f, 0.0f));

		// use 6th quad as the floor
		// make bigger
		tms[5]->scale(6.0f);
		// put in floor position
		tms[5]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), -90.0f);
		// center
		tms[5]->translate(V3(-100.0f, 0.0f, 80.0f));

		isA3Init = true;
	}

	const static V3 lookAtPoint(20.0f, 20.0f, -20.0f); // cube centroid
	const static V3 up(0.0f, 1.0f, 0.0f);
	V3 viewDirection = ppcLerp0->getViewDir();

	// 10 second video at 30 fps
#ifdef _MAKE_VIDEO_
	string pngFilename;
	int si = 0;
#endif
	for (float steps = 0.0f; steps < 360.0f; steps += 1.2f) {

		// rotate view direction
		V3 rotVD = viewDirection;
		rotVD.rotateThisVectorAboutDirection(V3(0.0f, 1.0f, 0.0f), steps);

		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		//ppc->positionAndOrient((lookAtPoint - (rotVD * 180.0f)), lookAtPoint, up); // also works

		// clear screen
		fb->set(0xFFFFFFFF);
		fb->clearZB(0.0f);

		// draw floor
		tms[5]->drawTextured(*fb, *ppc, *texObjects[5]);

		// draw textured cube
		for (int i = 0; i < 5; i++) {
			tms[i]->drawTextured(*fb, *ppc, *texObjects[i]);
		}

		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}
	return;
}

void Scene::testTexture(void)
{
	if (!(this->isTextureInit)) {

		cleanForNewScene();
		tms[0] = new TMesh();

		// create textured quad
		ppc->moveForward(-200.0f);

		tms[0]->createQuadTMesh(true); // enables tiling

		//texObject = new Texture("pngs\\Woven_flower_pxr128.png"); // test simple texturing
		texObjects[0] = new Texture("pngs\\Macbeth_color_checker_pxr128.png"); // test tiling
		this->isTextureInit = true;
	}
	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	fb->clearZB(0.0f);
	//drawTMesh(*tms[0], *fb, *ppc, false);
	tms[0]->drawTextured(*fb, *ppc, *texObjects[0]);
	// draw original for comparison
	fb->loadFromPng("pngs\\Macbeth_color_checker_pxr128.png");

	fb->redraw();
	return;
}

void Scene::testBilTexLookup(void)
{
	if (!isTestBilTexLookupInit) {

		cleanForNewScene();
		// prepare 6 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();
		tms[4] = new TMesh();
		tms[5] = new TMesh();

		// look at quads full frontal
		ppc->moveForward(-180.0f);
		ppc->moveUp(100.0f);
		ppc->moveRight(20.0f);
		ppc->tilt(-25);
		// save initial camera settings into ppcLerp0
		ppcLerp0 = new PPC(*ppc);

		// enable tiling for all the quads
		tms[0]->createQuadTMesh(false);
		tms[1]->createQuadTMesh(false);
		tms[2]->createQuadTMesh(false);
		tms[3]->createQuadTMesh(false);
		tms[4]->createQuadTMesh(false);
		tms[5]->createQuadTMesh(true);

		// load five different textures for demoing
		texObjects[0] = new Texture("pngs\\Woven_flower_pxr128.png");
		texObjects[1] = new Texture("pngs\\Macbeth_color_checker_pxr128.png");

		// use 5 quads to cover five faces of a 3D cube
		// tms[0] is the front face
		// tms[1] is the left face
		tms[1]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), -90.0f);
		// push in place
		tms[1]->translate(V3(0.0f, 0.0f, -40.0f));
		// tms[2] is the right face
		tms[2]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), 90.0f);
		// push in place
		tms[2]->translate(V3(40.0f, 0.0f, 0.0f));
		// tms[3] is the back face
		tms[3]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(0.0f, 1.0f, 0.0f), 180.0f);
		// push in place
		tms[3]->translate(V3(40.0f, 0.0f, -40.0f));
		// tms[4] is the top face
		tms[4]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), -90.0f);
		// push in place
		tms[4]->translate(V3(0.0f, 40.0f, 0.0f));

		// use 6th quad as the floor
		// make bigger
		tms[5]->scale(6.0f);
		// put in floor position
		tms[5]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), -90.0f);
		// center
		tms[5]->translate(V3(-100.0f, 0.0f, 80.0f));

		isTestBilTexLookupInit = true;
	}

	const static V3 lookAtPoint(20.0f, 20.0f, -20.0f); // cube centroid
	const static V3 up(0.0f, 1.0f, 0.0f);
	V3 viewDirection = ppcLerp0->getViewDir();

	// 10 second video at 30 fps
	for (float steps = 0.0f; steps < 360.0f; steps += 1.2f) {

		// rotate view direction
		V3 rotVD = viewDirection;
		rotVD.rotateThisVectorAboutDirection(V3(0.0f, 1.0f, 0.0f), steps);

		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		//ppc->positionAndOrient((lookAtPoint - (rotVD * 180.0f)), lookAtPoint, up); // also works

		// clear screen
		fb->set(0xFFFFFFFF);
		fb->clearZB(0.0f);

		// draw floor
		tms[5]->drawTextured(*fb, *ppc, *texObjects[1]);

		// draw textured cube
		for (int i = 0; i < 5; i++) {
			tms[i]->drawTextured(*fb, *ppc, *texObjects[1]);
		}

		fb->redraw();
		Fl::check();
	}
	return;
}

void Scene::testSprites(void)
{
	if (!(this->isSpriteTestInit)) {

		cleanForNewScene();
		tms[0] = new TMesh();

		// set up camera
		ppc->moveForward(-200.0f);
		ppc->moveRight(20.0f);
		ppc->moveUp(20.0f);

		// test textured quad
		tms[0]->createQuadTMesh(false); // no tiling

		//texObjects[0] = new Texture("pngs\\Decal_12.png");
		texObjects[0] = new Texture("pngs\\T_Explosion_SubUV.png");
		texObjects[1] = new Texture("pngs\\Bride-sprite-sheet.png");
		texObjects[2] = new Texture("pngs\\FireTorchSpriteAtlas.png");
		this->isSpriteTestInit = true;
	}
	// draw explosion animated sprite
	unsigned int subRows = 6;
	unsigned int subColumns = 6;
	unsigned int timesDrawn;
	for (int i = (subRows - 1); i >= 0; i--) {
		for (unsigned int j = 0; j < subColumns; j++) {
			// clear screen for sprite
			fb->set(0x00000000);
			// clear zBuffer
			fb->clearZB(0.0f);
			tms[0]->drawSprite(
				*fb,
				*ppc,
				*texObjects[0],
				j, (unsigned int) i, subColumns, subRows);
			fb->redraw();
			Fl::check();
		}
	}

	// draw torch animated sprite
	tms[0]->scale(1.0);
	subRows = 6;
	subColumns = 6;
	for (timesDrawn = 0; timesDrawn < 2; timesDrawn++) {
		for (int i = (subRows - 1); i >= 0; i--) {
			for (unsigned int j = 0; j < subColumns; j++) {
				// clear screen for sprite
				fb->set(0x00000000);
				// clear zBuffer
				fb->clearZB(0.0f);
				tms[0]->drawSprite(
					*fb,
					*ppc,
					*texObjects[2],
					j, (unsigned int)i, subColumns, subRows);
				fb->redraw();
				Fl::check();
			}
		}
	}
	// draw Braid animated sprite
	tms[0]->scale(0.5);
	subRows = 4;
	subColumns = 7;
	for (timesDrawn = 0; timesDrawn < 2; timesDrawn++) {
		for (int i = (subRows - 1); i >= 0; i--) {
			for (unsigned int j = 0; j < subColumns; j++) {
				// clear screen for sprite
				fb->set(0x00000000);
				// clear zBuffer
				fb->clearZB(0.0f);
				tms[0]->drawSprite(
					*fb,
					*ppc,
					*texObjects[1],
					j, (unsigned int)i, subColumns, subRows);
				fb->redraw();
				Fl::check();
			}
		}
	}
	return;
}

void Scene::renderFBAs3DPointCloud(void)
{
	// fbToRender and fbPPC could by parameters passed in 
	const SWFrameBuffer *const fbToRender = this->fb;
	const PPC *const fbPPC = this->ppc;

	// 3D point cloud is rendered using this camera and
	// an auxiliary frame buffer
	PPC currPPC(*fbPPC);
	currPPC.moveForward(80.0f);
	currPPC.moveUp(50.0f);
	currPPC.tilt(-20.0f);
	if (fbAux) {
		delete fbAux;
		fbAux = nullptr;
	}
	fbAux = new SWFrameBuffer(u0 + 200, v0 + 200, K_W, K_H);
	fbAux->label("Third person FB");
	fbAux->show();
	fbAux->clearZB(0.0f);
	fbAux->set(0xFFFFFFFF);

	for (int v = 0; v < fbToRender->getHeight(); v++) {
		for (int u = 0; u < fbToRender->getWidth(); u++) {
			int uv = (fbToRender->getHeight() - v - 1) * fbToRender->getWidth() + u;

			if (fbToRender->getZbAt(uv) == 0.0f)
				continue;

			// get projected point from fbToRender and unproject to get 3D point back
			V3 projP = V3(.5f + (float)u, .5f + (float)v, fbToRender->getZbAt(uv));
			V3 P = fbPPC->unproject(projP);

			// project gotten 3D point into currFB for visualization
			// using same color as original point
			V3 projP2;
			if (!currPPC.project(P, projP2))
				continue;
			V3 currCol;
			currCol.setFromColor(fbToRender->getPixAt(uv));
			fbAux->setIfOneOverWCloser(projP2, currCol);
		}
	}

	fbAux->redraw();
}

void Scene::testShadowMap(void)
{
	if (!isShadowMapTestInit) {

		cleanForNewScene();
		// prepare 2 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();

		// look at quads full frontal
		ppc->moveForward(-400.0f);
		ppc->moveUp(120.0f);
		ppc->moveRight(100.0f);

		// set up point light
		light->setAmbientK(0.4f);
		light->setMatColor(V3(1.0f, 0.0f, 0.0f));
		light->setPosition(ppc->getEyePoint());
		// this is needed by shadow map camera ot know
		// where to look since cubemap feature for point
		// lights is disabled by default for now
		light->setDirection(V3(0.0f, 0.0f, -1.0f));

		// Create a plane TMesh and a teapot TMesh
		tms[0]->createQuadTMesh(false);
		tms[0]->scale(6.0f);
		tms[1]->loadBin("geometry/teapot1K.bin");
		tms[1]->translate(V3(130.0f, 100.0f, 50.0f));

		// load texture
		//texObjects[0] = new Texture("pngs\\Woven_flower_pxr128.png");

		// create shadow maps
		vector<TMesh *> tMeshArray;
		for (int j = 0; j < 2; j++) {
			tMeshArray.push_back(tms[j]);
		}
		light->buildShadowMaps(tMeshArray, true);

		isShadowMapTestInit = true;
	}

	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	// enable shadow mapping
	drawTMesh(*tms[0], *fb, *ppc, false, true, false);
	drawTMesh(*tms[1], *fb, *ppc, false, true, false);
	fb->redraw();
	return;
}

void Scene::testTexProj(void)
{
	if (!isTexProjectTestInit) {

		cleanForNewScene();
		// prepare 2 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();

		// look at quads full frontal
		ppc->moveForward(-400.0f);
		ppc->moveUp(120.0f);
		ppc->moveRight(100.0f);

		// set up light
		light->setAmbientK(0.4f);
		light->setMatColor(V3(1.0f, 0.0f, 0.0f));
		light->setPosition(ppc->getEyePoint());
		// this is needed by shadow map camera ot know
		// where to look since cubemap feature for point
		// lights is disabled by default for now
		light->setDirection(V3(0.0f, 0.0f, -1.0f));

		// Create a plane TMesh and a teapot TMesh
		tms[0]->createQuadTMesh(false);
		tms[0]->scale(6.0f);
		tms[1]->loadBin("geometry/teapot1K.bin");
		tms[1]->translate(V3(130.0f, 100.0f, 50.0f));

		// load texture
		//texObjects[0] = new Texture("pngs\\Woven_flower_pxr128.png");

		// create a light projector and set it up
		//lightProjector = new LightProjector("pngs\\Woven_flower_pxr128.png");
		//lightProjector = new LightProjector("pngs\\Macbeth_color_checker_pxr128.png");
		//lightProjector = new LightProjector("pngs\\White_brick_block_pxr128.png");
		//lightProjector = new LightProjector("pngs\\Decal_12.png"); // test aplha
		//lightProjector = new LightProjector("pngs\\T_Explosion_SubUV_alpha.png"); // test aplha
		lightProjector = new LightProjector("pngs\\banskyGreen.png"); // test aplha best tone so far
		lightProjector->setPosition(ppc->getEyePoint());
		lightProjector->setDirection(ppc->getViewDir());

		// create shadow maps
		vector<TMesh *> tMeshArray;
		for (int j = 0; j < 2; j++) {
			tMeshArray.push_back(tms[j]);
		}
		light->buildShadowMaps(tMeshArray);
		lightProjector->buildShadowMaps(tMeshArray, true);

		isTexProjectTestInit = true;
	}

	// clear screen
	fb->set(0xFFFFFFFF);
	//fb->set(0x00000000);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	// enable shadow mapping for the quad
	drawTMesh(*tms[0], *fb, *ppc, false, true, true);
	drawTMesh(*tms[1], *fb, *ppc, false, true, true);
	fb->redraw();
	return;
}

void Scene::testA4Demo(void)
{
	V3 shadowmapDirection;
	V3 lightPosRot, lightAxisRot, lightPos;
	// 20 second video at 30 fps
#ifdef _MAKE_VIDEO_
	string pngFilename;
	int si = 0;
#endif
	if (!isA4DemoInit) {

		cleanForNewScene();
		// prepare 4 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();

		// set up initial camera view
		//ppc->moveForward(-200.0f);
		ppc->moveForward(-240.0f);
		ppc->moveUp(100.0f);
		ppc->moveRight(150.0f);
		ppc->tilt(-15);

		tms[0]->createQuadTMesh(true); // enable tiling for floor quad
		tms[1]->loadBin("geometry/happy4.bin");
		//tms[1]->loadBin("geometry/happy2.bin"); // high res version (not ready for it, takes too long)
		tms[2]->loadBin("geometry/teapot1K.bin");
		//tms[2]->loadBin("geometry/teapot57K.bin"); // high res version (not ready for it, takes too long)
		tms[3]->loadBin("geometry/auditorium.bin");
		tms[3]->rotateAboutAxis(tms[3]->getCenter(), V3(1.0f, 0.0f, 0.0f), -90.0f);

		// scale happy mesh to be same scale as teapots
		AABB teapotAABB = tms[2]->getAABB();
		tms[1]->setToFitAABB(teapotAABB);
		tms[3]->setToFitAABB(teapotAABB);

		// load floor texture
		texObjects[0] = new Texture("pngs\\American_walnut_pxr128.png");

		// use quad mesh as the floor
		// make bigger
		tms[0]->scale(7.0f);
		// put in floor position
		tms[0]->rotateAboutAxis(V3(0.0f, 0.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), -90.0f);
		// position happy tmesh at the center of floor
		tms[1]->translate(V3(200.0f, 23.0f, -140.0f));
		// position teapot tmesh at the center of floor
		tms[2]->translate(V3(120.0f, 0.0f, -200.0f));
		// translate auditorium to camera view
		tms[3]->translate(tms[1]->getCenter());

		// set up light, build this point light with a special HFOV for shadow maps
		delete light;
		light = nullptr;
		light = new Light(true, 90.0f);
		light->setAmbientK(0.4f);

		// create a light projector
		lightProjector = new LightProjector("pngs\\banskyGreen.png"); // test aplha best tone so far

		isA4DemoInit = true;
	}

	// step 1) showcase shadow mapping

	vector<TMesh *> tMeshArray;
	// create tMesh array to buiild shadow map later
	// don't include auditorium here
	for (int j = 0; j < 3; j++) {
		tMeshArray.push_back(tms[j]);
	}

	// calculate scene's centroid
	V3 sceneCentroid;
	for (int j = 0; j < 3; j++) {
		sceneCentroid += tms[j]->getCenter();
	}
	sceneCentroid = sceneCentroid / 3.0f;

	// initial light position
	light->setPosition(V3(280.0f, 0.0f, 0.0f));
	// set up axis to rotate light about
	lightAxisRot = V3(280.0f, 0.0f, -280.0f) - V3(0.0f, 0.0f, 0.0f);
	lightAxisRot.normalize();
	lightPos = light->getPosition();

	//14s -> 30f * 14s = 420 -> 180 / 420 -> 0.4285
	for (float steps = 0.0f; steps < 180.0f; steps += 0.4285f) {
		// clear screen and depth buffer
		fb->set(0x00000000);
		fb->clearZB(0.0f);
		// rotate light position
		lightPosRot = lightPos;
		lightPosRot.rotateThisPointAboutAxis(V3(0.0f, 0.0f, 0.0f), lightAxisRot, -steps);
		light->setPosition(lightPosRot);
		// figure out shadow map direction so it captures most of the scene
		shadowmapDirection = sceneCentroid - light->getPosition();
		shadowmapDirection.normalize();
		light->setDirection(shadowmapDirection);
		// since light position changed, update shadow maps
		light->buildShadowMaps(tMeshArray, true);
		// draw light for light position debug
		light->draw(*fb, *ppc, V3(253.0f / 255.0f, 184.0f / 255.0f, 19.0f/255.0f));
		// draw floor mesh in lit + texture mode

		light->setMatColor(V3(1.0f, 1.0f, 1.0f)); // set to white to modulate against texture
		tms[0]->drawLit(*fb, *ppc, *light, lightProjector,
			texObjects[0], false, true, false);

		// draw teapot mesh and happy mesh
		light->setMatColor(V3(104.0f/255.0f, 108.0f/255.0f, 94.0f/255.0f)); // concrete color
		//drawTMesh(*tms[1], *fb, *ppc, false, true, false);
		tms[1]->drawLit(*fb, *ppc, *light, lightProjector,
			nullptr, false, true, false);
		tms[2]->drawLit(*fb, *ppc, *light, lightProjector,
			nullptr, false, true, false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// step 2) showcase projective texturing

	// set up initial camera inside auditorium looking at seats
	ppc->loadCameraFromFile("camera_saves/A4DemoAudCamera.txt");

	// set up projector
	tMeshArray.clear();
	tMeshArray.push_back(tms[3]);
	lightProjector->setPosition(ppc->getEyePoint());
	lightProjector->setDirection(ppc->getViewDir());
	lightProjector->buildShadowMaps(tMeshArray, false);

	// set up light
	light->setPosition(ppc->getEyePoint());
	// shadow mapping is not used for this scene but it can
	// (although it doesn't add anything visually)
	lightProjector->setDirection(ppc->getViewDir());
	lightProjector->buildShadowMaps(tMeshArray, false);
	// light color needs to be white in order to modulate by colors of tmesh
	light->setMatColor(V3(1.0f , 1.0f, 1.0f));

	// set up camera interpolation
	static int i = 0;
	//6s -> 30f * 6s = 180
	static int n = 180;
	ppcLerp0 = new PPC("camera_saves\\A4DemoAudCamera.txt");
	ppcLerp1 = new PPC("camera_saves\\A4DemoAudCameraEnd.txt");
	for (i = 0; i < n; i++) {

		// clear screen
		fb->set(0xFFFFFFFF);
		// clear zBuffer
		fb->clearZB(0.0f);

		ppc->setByInterpolation(*ppcLerp0, *ppcLerp1, i, n);

		// draw auditorium mesh in lit mode + colors + shadowmap + projective texturing
		tms[3]->drawLit(*fb, *ppc, *light, lightProjector,
			nullptr, true, false, true);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	return;
}

void Scene::testA4DemoExtra(void)
{
	V3 shadowmapDirection;
	V3 lightPosRot, lightAxisRot, lightPos;
#ifdef _MAKE_VIDEO_
	string pngFilename;
	int si = 0;
#endif

	if (!isA4DemoExtraInit) {

		cleanForNewScene();
		// prepare 3 meshes
		tms[0] = new TMesh();
		tms[1] = new TMesh();

		// set up initial camera view inside auditorium looking at seats
		ppc->loadCameraFromFile("camera_saves/A4MagicTrickCamera.txt");

		tms[0]->loadBin("geometry/teapot1K.bin");
		tms[1]->loadBin("geometry/auditorium.bin");
		tms[1]->rotateAboutAxis(tms[1]->getCenter(), V3(1.0f, 0.0f, 0.0f), -90.0f);

		// scale happy mesh to be same scale as teapots
		AABB teapotAABB = tms[0]->getAABB();
		tms[1]->setToFitAABB(teapotAABB);
		tms[0]->scale(0.25f);

		// translate auditorium to known location
		tms[1]->translate(V3(200.0f, 50.0f, -140.0f));

		// create a light projector and set it up, texture loaded does not 
		// matter since its going to use its own texture from the color shadow map
		lightProjector = new LightProjector("pngs\\banskyGreen.png");

		isA4DemoExtraInit = true;
	}
	// 10 second vide at 30fps -> (245.0-160)/300 = 0.28333
	float delta = 0.28333f;//5.0f;
	V3 lookAtTeapot;

	vector<TMesh *> tMeshArray;
	// create tMesh array to for shadow map. Its very
	// important that this array does not contain the 
	// teapot
	tMeshArray.push_back(tms[1]);

	// set up texture projector above audience heads
	lightProjector->setPosition(ppc->getEyePoint());
	V3 auxVec = lightProjector->getPosition();
	auxVec[1] += 2.0;
	lightProjector->setPosition(auxVec);
	
	// position teapot tmesh at front of audience in auditorium
	tms[0]->translate(V3(160.0f, 60.0f, -90.0f)); // starting point

	// what audience sees without the effect
	for (float step = 160.0f; step < 245.0f; step += delta) {
		// clear screen
		fb->set(0xFFFFFFFF);
		// clear zBuffer
		fb->clearZB(0.0f);

		// draw auditorium mesh in color interpolation mode 
		tms[1]->drawFilledFlatBarycentric(*fb, *ppc);
		// draw teapot in color interpolation mode
		tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
		tms[0]->translate(V3(delta, 0.0f, 0.0f));
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// reset position teapot tmesh at front of audience in auditorium
	tms[0]->loadBin("geometry/teapot1K.bin");
	tms[0]->scale(0.25f);
	tms[0]->translate(V3(160.0f, 60.0f, -90.0f)); // starting point

	// what the projector projects
	for (float step = 160.0f; step < 245.0f; step += delta) {
		// clear screen
		fb->set(0xFFFFFFFF);
		// clear zBuffer
		fb->clearZB(0.0f);

		// rendere scene from projector perspective
		ppc->positionAndOrient(
			lightProjector->getPosition(),
			tms[0]->getCenter(),
			V3(0.0f, 1.0f, 0.0f));

		// draw auditorium mesh in color interpolation mode
		tms[1]->drawFilledFlatBarycentric(*fb, *ppc);
		// move teapot
		//tms[0]->drawFilledFlatBarycentric(*fb, *ppc);
		tms[0]->translate(V3(delta, 0.0f, 0.0f));
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// reset camera
	// set up initial camera view inside auditorium looking at seats
	ppc->loadCameraFromFile("camera_saves/A4MagicTrickCamera.txt");

	// reset position teapot tmesh at front of audience in auditorium
	tms[0]->loadBin("geometry/teapot1K.bin");
	tms[0]->scale(0.25f);
	tms[0]->translate(V3(160.0f, 60.0f, -90.0f)); // starting point

	// what audience sees with the effect
	for (float step = 160.0f; step < 245.0f; step += delta) {
		// clear screen
		fb->set(0xFFFFFFFF);
		// clear zBuffer
		fb->clearZB(0.0f);

		// set up projector
		lookAtTeapot = tms[0]->getCenter() - lightProjector->getPosition();
		lookAtTeapot.normalize();
		lightProjector->setDirection(lookAtTeapot);
		// last argument equals false to indicate that we want to build the shadow
		// map using interpolated colors as opposed to flat mode as its usually done
		lightProjector->buildShadowMaps(tMeshArray, false, false);

		// draw auditorium mesh in color interpolation mode since color
		// already provides lighting cues. Adding a light makes things look too
		// bright right now since I hanve't implemented the distance attenuation 
		// for point lights.
		tms[1]->drawFilledFlatBarycentric(*fb, *ppc);
		// draw teapot in stealth mode (with lighting turned off)
		tms[0]->drawStealth(*fb, *ppc, nullptr, *lightProjector,
			nullptr, false, false);
		tms[0]->translate(V3(delta, 0.0f, 0.0f));
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}
	return;
}

void Scene::testCubeMapFaces(void)
{
	CubeMap cubeMap("pngs\\uffizi_cross.png");
	cleanForNewScene();
	static unsigned int i = 0;
	fb->loadFromTexture(*(cubeMap.getCubeFace(i % 6)));
	fb->redraw();
	i++;
	return;
}

void Scene::testCubeMapReflection(void)
{
	const float speedFactor = 0.01f;
	CubeMap cubeMap("pngs\\uffizi_cross.png");
	if (!isTestCMReflectInit) {

		cleanForNewScene();

		tms[0] = new TMesh();
		tms[1] = new TMesh();

		// load texture
		texObjects[0] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");

		// test environment mapping technique on teapot
		tms[0]->loadBin("geometry/teapot1K.bin");
		tms[0]->translate(V3(10.0f, -10.0f, 0.0f));
		tms[0]->scale(1.0);
		tms[1]->createQuadTMesh(true);
		tms[1]->scale(2.5);
		tms[1]->translate(V3(-50.0f, -40.0f, 0.0f));

		// set ppc with a HFOV big enough for env map
		delete ppc;
		ppc = nullptr;
		ppc = new PPC(90.0f, K_W, K_H);

		ppc->moveForward(-200.0f);

		isTestCMReflectInit = true;
	}

	const V3 lookAtPoint(tms[0]->getCenter()); // teapot centroid
	const V3 up(0.0f, 1.0f, 0.0f);
	V3 viewDirection = ppc->getViewDir();
	// rotate view direction
	V3 rotVD = viewDirection;
	rotVD.rotateThisVectorAboutDirection(V3(0.0f, 1.0f, 0.0f), mouseDeltaX * speedFactor);
	rotVD.rotateThisVectorAboutDirection(V3(1.0f, 0.0f, 0.0f), mouseDeltaY * speedFactor);
	// set up the look at cube camera (dolly camera setup)
	ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
	// roll needs to be done independently from cam dolly at the end
	ppc->roll(-(mouseDeltaZ * speedFactor));

	fb->drawEnvironmentMap(cubeMap, *ppc);
	fb->clearZB(0.0f);
	tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, true);
	tms[1]->drawReflective(cubeMap, *fb, *ppc, texObjects[0], false);
	fb->redraw();
	return;
}

void Scene::testCubeMapRefraction(void)
{
	const float nl = 1.0f; // refraction index of air
	const float nt = 1.51f; // refraction index of window glass
	//const float nt = 2.42f; // refraction index of diamond
	//const float nt = 1.33f; // refraction index of water
	CubeMap cubeMap("pngs\\uffizi_cross.png");
	if (!isTestCMRefractInit) {

		cleanForNewScene();

		tms[0] = new TMesh();
		tms[1] = new TMesh();

		// load texture
		texObjects[0] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");

		// test environment mapping technique on teapot
		tms[0]->loadBin("geometry/teapot1K.bin");
		tms[0]->translate(V3(10.0f, -10.0f, 0.0f));
		tms[0]->scale(1.0);
		tms[1]->createQuadTMesh(true);
		tms[1]->scale(2.5);
		tms[1]->translate(V3(-50.0f, -40.0f, 0.0f));

		// set ppc with a HFOV big enough for env map
		delete ppc;
		ppc = nullptr;
		ppc = new PPC(90.0f, K_W, K_H);

		ppc->moveForward(-200.0f);

		isTestCMRefractInit = true;
	}
	fb->drawEnvironmentMap(cubeMap, *ppc);
	fb->clearZB(0.0f);
	tms[0]->drawRefractive(nl, nt, cubeMap, *fb, *ppc, nullptr, false);
	//tms[1]->drawRefractive(nl, nt, cubeMap, *fb, *ppc, texObjects[0], false);
	//tms[1]->drawRefractive(nl, nt, cubeMap, *fb, *ppc, nullptr, false);
	fb->redraw();
	return;
}

void Scene::a5Demo(void)
{
#ifdef _MAKE_VIDEO_
	string pngFilename;
	int si = 0;
#endif
	const float nl = 1.0f; // refraction index of air
	const float nt = 1.51f; // refraction index of window glass
	CubeMap cubeMap("pngs\\uffizi_cross.png");
	cleanForNewScene();
	tms[0] = new TMesh();
	tms[1] = new TMesh();

	// load texture
	texObjects[0] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");

	// test environment mapping technique on teapot
	tms[0]->loadBin("geometry/teapot1K.bin");
	//tms[0]->loadBin("geometry/teapot57K.bin"); // HD model
	tms[0]->translate(V3(10.0f, -10.0f, 0.0f));
	tms[0]->scale(1.0);
	tms[1]->createQuadTMesh(true);
	tms[1]->scale(2.5);
	tms[1]->translate(V3(-50.0f, -60.0f, 0.0f));
	tms[1]->rotateAboutAxis(tms[1]->getCenter(), V3(1.0f, 0.0f, 0.0f), 90.0f);

	// set ppc with a HFOV big enough for env map
	delete ppc;
	ppc = nullptr;
	ppc = new PPC(90.0f, K_W, K_H);

	const V3 lookAtPoint(tms[0]->getCenter()); // teapot centroid
	const V3 up(0.0f, 1.0f, 0.0f);
	V3 viewDirection = ppc->getViewDir();

	// pans horizontally showcasing reflections with color tMesh
	// takes 5 seconds -> 30*5 = 150 frames -> 90/150 = 0.6
	for (float steps = 0.0f; steps < 90.0f; steps += 0.6f) {

		// rotate view direction
		V3 rotVD = viewDirection;
		rotVD.rotateThisVectorAboutDirection(V3(0.0f, 1.0f, 0.0f), steps);
		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, true);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// tilts vertically showcasing reflections at bottom of chromed teapot
	// takes 5 seconds -> 30*5 = 150 frames -> 90/150 = 0.6
	for (float steps = 0.0f; steps < 90.0f; steps += 0.6f) {

		// rotate view direction
		V3 rotVD = viewDirection;
		rotVD.rotateThisVectorAboutDirection(V3(1.0f, 0.0f, 0.0f), steps);
		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// tilts vertically showcasing reflections at top of chromed teapot
	// takes 5 seconds -> 30*5 = 150 frames -> 90/150 = 0.6
	for (float steps = 0.0f; steps < 90.0f; steps += 0.6f) {

		// rotate view direction
		V3 rotVD = viewDirection;
		rotVD.rotateThisVectorAboutDirection(V3(1.0f, 0.0f, 0.0f), -steps);
		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, true);
		// draw textured quad (useful when implementing billboard reflection raytracing)
		//tms[1]->drawReflective(cubeMap, *fb, *ppc, texObjects[0], false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// rolls to the right showcasing reflections on colored teapot
	// takes 2.5 seconds -> 30*2.5 = 75 frames -> 45/75 = 0.6
	ppc->positionRelativeToPoint(lookAtPoint, viewDirection, up, 180.0f);
	for (float steps = 0.0f; steps < 45.0f; steps += 0.6f) {

		// roll the camera
		ppc->roll(0.6f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// rolls to the left showcasing reflections on colored teapot
	// takes 2.5 seconds -> 30*2.5 = 75 frames -> 45/75 = 0.6
	for (float steps = 0.0f; steps < 45.0f; steps += 0.6f) {

		// roll the camera
		V3 rotVD = viewDirection;
		ppc->roll(-0.6f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawReflective(cubeMap, *fb, *ppc, nullptr, false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	// rotates around an axis showcasing refractions on a glass teapot
	// takes 10 seconds -> 30*10 = 300 frames -> 0.6
	for (float steps = 0.0f; steps < 180.0f; steps += 0.6f) {
		
		// rotate view direction
		V3 rotVD = viewDirection;
		V3 dir = V3(-1.0f, 1.0f, 0.0f);
		dir.normalize();
		rotVD.rotateThisVectorAboutDirection(dir, steps);
		// set up the look at cube camera (dolly camera setup)
		ppc->positionRelativeToPoint(lookAtPoint, rotVD, up, 180.0f);
		// draw distant geometry
		fb->drawEnvironmentMap(cubeMap, *ppc);
		fb->clearZB(0.0f);
		// draw teapot
		tms[0]->drawRefractive(nl, nt, cubeMap, *fb, *ppc, nullptr, false);
		fb->redraw();
		Fl::check();
#ifdef _MAKE_VIDEO_
		pngFilename = string("movieFrame");
		pngFilename += std::to_string(si) + ".png";
		fb->saveAsPng(pngFilename);
		si++;
#endif
	}

	return;
}

void Scene::testFixedPipelineHW(void)
{
	if (!isTestFixedPipelineInit) {

		cleanForNewScene();

		// create fixed pipeline HW framebuffer
		fixedHwFb = new HWFixedPipeline(u0, v0, K_W, K_H);
		fixedHwFb->label("Fixed Pipeline HW Framebuffer");

		unsigned int tmsN = 5;
		unsigned int n;
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();
		tms[4] = new TMesh();

		tms[0]->createQuadTMesh(false);
		tms[0]->translate(V3(-100.0f, 50.0f, 0.0f));
		tms[1]->createQuadTMesh(false);
		tms[1]->translate(V3(20.0f, 50.0f, 0.0f));
		tms[2]->createQuadTMesh(true);
		tms[2]->translate(V3(-100.0f, 0.0f, 0.0f));
		tms[3]->createQuadTMesh(true);
		tms[3]->translate(V3(20.0f, 0.0f, 0.0f));
		tms[4]->loadBin("geometry/teapot1K.bin");
		tms[4]->disableTexCoords(); // they don't look good
		for (n = 0; n < tmsN; n++) {
			fixedHwFb->registerTMesh(tms[n]);
		}

		// load five different textures for demoing
		unsigned int texObjectsN = 6;
		texObjects[0] = new Texture("pngs\\White_brick_block_pxr128.png");
		texObjects[1] = new Texture("pngs\\Macbeth_color_checker_pxr128.png"); // test tiling
		texObjects[2] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");
		texObjects[3] = new Texture("pngs\\Brown_staggered_pxr128.png");
		texObjects[4] = new Texture("pngs\\Woven_flower_pxr128.png");
		texObjects[5] = new Texture("pngs\\American_walnut_pxr128.png");
		for (n = 0; n < texObjectsN; n++) {
			fixedHwFb->registerTexture(texObjects[n]);
		}

		// assign tMesh to texture mappings
		fixedHwFb->assignTMeshTexture(0, 0);
		fixedHwFb->assignTMeshTexture(1, 1);
		fixedHwFb->assignTMeshTexture(2, 2);
		fixedHwFb->assignTMeshTexture(3, 3);

		V3 center = tms[4]->getCenter();
		ppc->moveForward(-200.0f);
		ppc->positionAndOrient(ppc->getEyePoint(), center, V3(0.0f, 1.0f, 0.0f));
		ppc->moveUp(10.0f);
		fixedHwFb->registerPPC(ppc);

		fixedHwFb->show();
		isTestFixedPipelineInit = true;
	}
	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	drawTMesh(*tms[0], *fb, *ppc, true);

	fb->redraw();
	fixedHwFb->redraw();
	return;
}

void Scene::testProgrPipelineHW(void)
{
	if (!isTestProgrPipelineInit) {

		cleanForNewScene();

		// create progr pipeline HW framebuffer
		progrHwFb = new HWProgrPipeline(u0, v0, K_W, K_H);
		progrHwFb->label("Programmable Pipeline HW Framebuffer");

		unsigned int tmsN = 5;
		unsigned int n;
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();
		tms[3] = new TMesh();
		tms[4] = new TMesh();

		tms[0]->createQuadTMesh(false);
		tms[0]->translate(V3(-100.0f, 50.0f, 0.0f));
		tms[1]->createQuadTMesh(false);
		tms[1]->translate(V3(20.0f, 50.0f, 0.0f));
		tms[2]->createQuadTMesh(true);
		tms[2]->translate(V3(-100.0f, 0.0f, 0.0f));
		tms[3]->createQuadTMesh(true);
		tms[3]->translate(V3(20.0f, 0.0f, 0.0f));
		tms[4]->loadBin("geometry/teapot1K.bin");
		tms[4]->disableTexCoords(); // they don't look good
		for (n = 0; n < tmsN; n++) {
			progrHwFb->registerTMesh(tms[n]);
		}

		// load five different textures for demoing
		unsigned int texObjectsN = 6;
		texObjects[0] = new Texture("pngs\\White_brick_block_pxr128.png");
		texObjects[1] = new Texture("pngs\\Macbeth_color_checker_pxr128.png"); // test tiling
		texObjects[2] = new Texture("pngs\\Alloy_diamond_plate_pxr128.png");
		texObjects[3] = new Texture("pngs\\Brown_staggered_pxr128.png");
		texObjects[4] = new Texture("pngs\\Woven_flower_pxr128.png");
		texObjects[5] = new Texture("pngs\\American_walnut_pxr128.png");
		for (n = 0; n < texObjectsN; n++) {
			progrHwFb->registerTexture(texObjects[n]);
		}

		// assign tMesh to texture mappings
		progrHwFb->assignTMeshTexture(0, 0);
		progrHwFb->assignTMeshTexture(1, 1);
		progrHwFb->assignTMeshTexture(2, 2);
		progrHwFb->assignTMeshTexture(3, 3);
		progrHwFb->assignTMeshTexture(4, 3);

		V3 center = tms[4]->getCenter();
		ppc->moveForward(-200.0f);
		ppc->positionAndOrient(ppc->getEyePoint(), center, V3(0.0f, 1.0f, 0.0f));
		ppc->moveUp(10.0f);
		progrHwFb->registerPPC(ppc);

		progrHwFb->show();
		isTestProgrPipelineInit = true;
	}
	// clear screen
	fb->set(0xFFFFFFFF);
	// clear zBuffer
	if (currentDrawMode == DrawModes::MODELSPACELERP)
		fb->clearZB(FLT_MAX);
	else
		fb->clearZB(0.0f);
	drawTMesh(*tms[0], *fb, *ppc, true);

	fb->redraw();
	progrHwFb->redraw();
	return;
}

void Scene::a6Demo(void)
{
	if (!isA6DemoInit) {

		cleanForNewScene();

		// create progr pipeline HW framebuffer
		reflectionshwFb = new HWReflections(u0, v0, K_W, K_H);
		reflectionshwFb->label("Programmable Pipeline HW Framebuffer for A6");

		unsigned int tmsN = 3;
		unsigned int n;
		tms[0] = new TMesh();
		tms[1] = new TMesh();
		tms[2] = new TMesh();

		tms[0]->loadBin("geometry/teapot1K.bin");
		tms[0]->disableTexCoords(); // they don't look good
		tms[1]->loadBin("geometry/happy4.bin");
		tms[2]->loadBin("geometry/happy4.bin");
		// scale happy mesh to be same scale as teapots
		AABB teapotAABB = tms[0]->getAABB();
		tms[1]->setToFitAABB(teapotAABB);
		tms[2]->setToFitAABB(teapotAABB);
		tms[1]->translate(V3(0.0f, 0.0f, -80.0f));
		tms[2]->translate(V3(-150.0f, 0.0f, 0.0f));
		for (n = 0; n < tmsN; n++) {
			reflectionshwFb->registerTMesh(tms[n]);
		}

		HWReflections *tempPointer = (HWReflections *) reflectionshwFb;
		tempPointer->setReflectorTMesh(0);

		V3 center = tms[0]->getCenter();
		ppc->moveForward(-400.0f);
		ppc->moveRight(-100.0f);
		ppc->positionAndOrient(ppc->getEyePoint(), center, V3(0.0f, 1.0f, 0.0f));
		ppc->moveUp(10.0f);
		reflectionshwFb->registerPPC(ppc);

		reflectionshwFb->show();
		isA6DemoInit = true;
	}
	// clear screen
	//fb->set(0xFFFFFFFF);
	// clear zBuffer
	//if (currentDrawMode == DrawModes::MODELSPACELERP)
		//fb->clearZB(FLT_MAX);
	//else
		//fb->clearZB(0.0f);
	//drawTMesh(*tms[0], *fb, *ppc, true);

	//fb->redraw();
	reflectionshwFb->redraw();
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

void Scene::regFuncForKbRedraw(Scenes newScene)
{
	currentScene = newScene;
}

void Scene::currentSceneRedraw(void)
{
	switch (currentScene) {
	case Scenes::DBG:
		this->dbgDraw();
		break;
	case Scenes::A1:
		this->testRot();
		break;
	case Scenes::A2:
		this->a2Demo();
		break;
	case Scenes::CAMLERP:
		this->testCameraLerp();
		break;
	case Scenes::CAMCONTROL:
		this->testCameraControl();
		break;
	case Scenes::A3:
		this->a3Demo();
		break;
	case Scenes::TEXTURE:
		this->testTexture();
		break;
	case Scenes::SPRITETEST:
		this->testSprites();
		break;
	case Scenes::SHADOWTEST:
		this->testShadowMap();
		break;
	case Scenes::TEXPROJTEST:
		this->testTexProj();
		break;
	case Scenes::REFLECTEST:
		this->testCubeMapReflection();
		break;
	case Scenes::REFRACTEST:
		this->testCubeMapRefraction();
		break;
	case Scenes::A4:
	case Scenes::A4EXTRA:
	default:
		dbgDraw();
		break; // this statement is optional for default
	}
}

void Scene::setDrawMode(int mode)
{
	switch (mode) {

	case 1: 
		currentDrawMode = DrawModes::DOTS;
		break;
	case 2:
		currentDrawMode = DrawModes::WIREFRAME;
		break;
	case 3:
		currentDrawMode = DrawModes::FLAT;
		break;
	case 4:
		currentDrawMode = DrawModes::SCREENSCAPELERP;
		break;
	case 5:
		currentDrawMode = DrawModes::MODELSPACELERP;
		break;
	case 6:
		currentDrawMode = DrawModes::LIT;
		break;
	default:
		currentDrawMode = DrawModes::WIREFRAME;
		break; // optional statement for default
	}
	if (currentScene == Scenes::DBG ||
		currentScene == Scenes::CAMLERP ||
		currentScene == Scenes::CAMCONTROL || 
		currentScene == Scenes::SHADOWTEST || 
		currentScene == Scenes::TEXPROJTEST ||
		currentScene == Scenes::A4 || 
		currentScene == Scenes::A4EXTRA) {
		currentSceneRedraw();
	}
}

void Scene::setMouseDelta(int mouseDeltaX, int mouseDeltaY)
{
	this->mouseDeltaX = mouseDeltaX;
	this->mouseDeltaY = mouseDeltaY;
}

void Scene::setMouseRoll(int mouseRoll)
{
	this->mouseDeltaZ = mouseRoll;
}
