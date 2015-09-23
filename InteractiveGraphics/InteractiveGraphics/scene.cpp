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

Scene::Scene() {

  // create user interface
  gui = new GUI();
  gui->show();

  // create SW framebuffer
  int u0 = 20;
  int v0 = 50;
  int w = 1280;
  int h = 720;
  fb = new FrameBuffer(u0, v0, w, h);
  fb->label("SW Framebuffer");
  fb->show();
  
  // position UI window
  gui->uiw->position(fb->w+u0 + 2*20, v0);

}

Scene::~Scene()
{
	delete fb;
	delete gui;
}

// function linked to the DBG GUI button for testing new features
void Scene::DBG() {

	fb->Set(0xFFFFFFFF);
	unsigned int pointColor = 0xFF0000FF;
	unsigned int originColor = 0xFFAABBFF;
	unsigned int circleColor = 0xFFCCDDFF;

	V3 point(300.0f, 180.0f, 0.0f);
	V3 axisOrigin(240.0f, 180.0f, 0.0f);
	V3 axisDirection(0.0f, 0.0f, 1.0f);
	M33 rotMatrix;
	rotMatrix.setRotationAboutX(45.0f);
	axisDirection = rotMatrix * axisDirection;

	float step = 1.0f;
	for (int stepsi = 0; stepsi <= 360; stepsi++) {
		point[0] = 300.0f;
		point[1] = 180.0f;
		point[2] = 0.0f;
		// clear screen
		fb->Set(0xFFFFFFFF);
		// test rectangle
		fb->DrawRectangle(200.0f, 300, 100.0, 50.0, originColor);
		// draw origin point
		fb->DrawCircle(axisOrigin[0], axisOrigin[1], 3.5f, originColor);
		// draw original point
		fb->DrawCircle(point[0], point[1], 3.5f, pointColor);

		point.rotateThisPointAboutAxis(axisOrigin, axisDirection, (float)stepsi);
		fb->DrawCircle(point[0], point[1], 3.5f, circleColor);
		fb->redraw();
		Fl::check();
	}

	//fb->SaveAsPng("test.png");
	fb->LoadFromPng("pngs\\vignette.png");
	fb->redraw();
	Fl::check();
	return;


	// circulat path
	/*fb->Set(0xFFFFFFFF);
	unsigned int pointColor = 0xFF0000FF;
	unsigned int originColor = 0xFFAABBFF;
	unsigned int circleColor = 0xFFCCDDFF;

	V3 point(300.0f, 180.0f, 0.0f);
	V3 origin(240.0f, 180.0f, 0.0f);

	float step = 1.0f;
	for (int stepsi = 0; stepsi <= 360; stepsi++) {
		point[0] = 300.0f;
		point[1] = 180.0f;
		point[2] = 0.0f;
		// clear screen
		fb->Set(0xFFFFFFFF);
		// test rectangle
		fb->DrawRectangle(200.0f, 300, 100.0, 50.0, originColor);
		// draw origin point
		//fb->SetSafe((int)origin[0], (int)origin[1], originColor);
		fb->DrawCircle(origin[0], origin[1], 3.5f, originColor);
		// draw original point
		//fb->SetSafe((int)point[0], (int)point[1], pointColor);
		fb->DrawCircle(point[0], point[1], 3.5f, pointColor);
		point.rotateAroundArbitAxis(origin, V3(0.0f, 0.0f, 1.0f), (float)stepsi);
		//fb->SetSafe((int)point[0], (int)point[1], pointColor);
		fb->DrawCircle(point[0], point[1], 3.5f, circleColor);
		fb->redraw();
		Fl::check();
	}
	return;*/

	/*fb->Set(0xFFFFFFFF);
	unsigned int pointColor = 0xFF0000FF;
	float xCoords[3] = { 200.0f, 250.0f, 300.0f };
	float yCoords[3] = { 300.0f, 50.0f, 250.0f };
	//fb->DrawCircle(xCoords[0], yCoords[0], 3.5f, pointColor);
	//fb->DrawCircle(xCoords[1], yCoords[1], 3.5f, pointColor);
	//fb->DrawCircle(xCoords[2], yCoords[2], 3.5f, pointColor);
	fb->DrawTriangle(xCoords, yCoords, pointColor);
	fb->redraw();
	return;*/

	M33 testM33InputOutput;
	cin >> testM33InputOutput;
	cout << testM33InputOutput;
	return;

	V3 testInputOutput;
	cin >> testInputOutput;
	cout << testInputOutput << endl;
	return;

	V3 v0(1.0f, 0.0f, 0.0f);
	V3 v1;

	//v1 = v0 * 100;
	cerr << v0[0] << ", " << v0[1] << ", " << v0[2] << endl;
	cerr << v1[0] << ", " << v1[1] << ", " << v1[2] << endl;
	v1 = 500 * v0;
	cerr << v0[0] << ", " << v0[1] << ", " << v0[2] << endl;
	cerr << v1[0] << ", " << v1[1] << ", " << v1[2] << endl;
	return;

  //V3 v0(1.0f, 0.0f, 0.0f);
  //V3 v1(0.0f, 1.0f, 0.0f);

  //M33 m;
  //m[0] = v0;
  //m[1] = v1;
  //m[2] = v0;

  //return;

  //v0[2] = 10.0f;
  //
  //float v0v1 = v0*v1;
  //cerr << v0v1 << endl;
  //return;

  fb->Set(0xFFFFFFFF);
  float cuf = 135.3f;
  float cvf = 81.9f;
  float radius = 51.0f;
  unsigned int color = 0xFF0000FF;
  fb->DrawCircle(cuf, cvf, radius, color);
  fb->redraw();
  return;

  fb->SetCheckerboard(40, 0xFFAAAAAA, 0xFFFFFFFF);
  fb->redraw();
  return;

  fb->Set(0xFFFFFFFF);
  int u = fb->w/2;
  color = 0xFF000000;
  for (int v = 0; v < fb->h; v++) {
    fb->Set(u, v, color);
  }
  fb->redraw();
  return;


  color = 0xFF00FFFF;
  ((unsigned char*)(&color))[1] = 0x80;

  unsigned char red = ((unsigned char*)(&color))[0];
  unsigned char green = ((unsigned char*)(&color))[1];
  unsigned char blue = ((unsigned char*)(&color))[2];


  cerr << "red: " << (int) red << "; green: " << (int) green << "; blue: " << (int) blue << endl;

  fb->Set(color);
  fb->redraw();

}

void Scene::TestRot() {

	fb->Set(0xFFFFFFFF);
	unsigned int pointColor = 0xFF0000FF;
	unsigned int originColor = 0xFFAA0000;
	unsigned int movingPointColor1 = 0xFF00FF80;
	unsigned int movingPointColor2 = 0xFFFF8000;
	unsigned int breadCrumbColor1 = 0xFF00CC66;
	unsigned int breadCrumbColor2 = 0xFFCC6600;
	float pointRadius = 5.0f;

	V3 point, axisDirection;
	V3 originalPoint(fb->w / 2.0f + 300.0f, fb->h / 2.0f, 0.0f);
	V3 axisOrigin(fb->w / 2.0f, fb->h / 2.0f, 0.0f);
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
		fb->Set(0xFFFFFFFF);
		// draw origin point
		fb->DrawCircle(axisOrigin[0], axisOrigin[1], pointRadius, originColor);
		// draw original point
		fb->DrawCircle(point[0], point[1], pointRadius, pointColor);

		// draw rotating point about z-axis (makes a circle)
		point.rotateThisPointAboutAxis(axisOrigin, axisDirection, (float)stepsi);
		fb->DrawCircle(point[0], point[1], pointRadius, movingPointColor1);
		// store new breadcrumb
		if (stepsi % 5 == 0) {
			breadcrumbsCircle.push_back(point);
		}

		// draw rotating point about rotated z-axis (makes an ellipse)
		// reset point;
		point = originalPoint;
		axisDirection = rotMatrix * axisDirection;
		point.rotateThisPointAboutAxis(axisOrigin, axisDirection, (float)stepsi);
		fb->DrawCircle(point[0], point[1], pointRadius, movingPointColor2);
		// store new breadcrumb
		if (stepsi % 5 == 0) {
			breadcrumbsEllipse.push_back(point);
		}

		// draw circle breadcrums so far collected (gives nice hint of point trajectory)
		for (vector<V3>::iterator it = breadcrumbsCircle.begin(); it != breadcrumbsCircle.end(); ++it) {
			V3 &breadcrumb = *it;
			fb->DrawCircle(breadcrumb[0], breadcrumb[1], pointRadius/2.0f, breadCrumbColor1);
		}

		// draw ellipse breadcrums so far collected (gives nice hint of point trajectory)
		for (vector<V3>::iterator it = breadcrumbsEllipse.begin(); it != breadcrumbsEllipse.end(); ++it) {
			V3 &breadcrumb = *it;
			fb->DrawCircle(breadcrumb[0], breadcrumb[1], pointRadius / 2.0f, breadCrumbColor2);
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

void Scene::TestRaster() {

	fb->Set(0xFFFFFFFF);
	unsigned int triangleColor = 0xFF0000FF;
	unsigned int circleColor = 0xFFAA0000;
	unsigned int rectangleColor = 0xFF00FF80;

	float circleRadius = 150.0f;
	V3 circleCenter(0 - circleRadius*2.0f, fb->h / 2.0f, 0.0f);

	// move circle from left to right, into scene and out
	for (float steps = 0 - (circleRadius*2.0f);
			steps <= fb->w + 2*(circleRadius*2.0f);
			steps += 10.0f) {
		// clear screen
		fb->Set(0xFFFFFFFF);
		// draw circle
		fb->DrawCircle(	circleCenter[0] + steps,
						circleCenter[1], 
						circleRadius,
						circleColor);
		fb->redraw();
		Fl::check();
	}

	float rectangleWidth = 400.0f;
	float rectangleHeight = 200.0f;
	V3 lowerLeft(fb->w / 2.0f, 0.0f, 0.0f);

	// move rectangle from bottom to top, into scene and out
	for (float steps = 0;
		steps <= fb->h + rectangleHeight;
		steps += 5.0f) {
		// clear screen
		fb->Set(0xFFFFFFFF);
		// draw rectangle
		fb->DrawRectangle(	lowerLeft[0], 
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
	float xCoords[3] = { (float) fb->w,
						fb->w + (triangleWidth/2.0f),
						fb->w + triangleWidth };

	float yCoords[3] = { fb->h / 2.0f, 
						(fb->h / 2.0f) - triangleHeight, 
						(fb->h / 2.0f) - (triangleHeight/3.0f) };

	for (float steps = 0;
		steps <= fb->w + triangleWidth;
		steps += 0.5f)
	{
		// clear screen
		fb->Set(0xFFFFFFFF);
		// draw triangle
		xCoords[0] = xCoords[0] - steps;
		xCoords[1] = xCoords[1] - steps;
		xCoords[2] = xCoords[2] - steps;

		fb->DrawTriangle(xCoords, yCoords, triangleColor);
		fb->redraw();
		Fl::check();
	}
	return;
}