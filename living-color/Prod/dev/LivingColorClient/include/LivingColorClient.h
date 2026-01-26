#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"
#include "cinder/Serial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LCLed
{
public:
	LCLed();
	LCLed(vec2 _pos, int _id);
	LCLed(float _x, float _y, int _id);

	void show(bool isInside);
	int getId() const;
	vec2 getPos() const;

private:
	vec2 mPos;
	int mId;
};

class LivingColorClientApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

private:
	void setupCom(const string &name);
	void setupLEDs();

	void updateImageBuffers();
	void updateLeds();

	void drawLEDs();

	vector<LCLed> mLeds;
	vector<int> mSelected;
	
	vec2 mMousePos;
	Rectf mTestBounds;
	vector<cv::Point> mTestPoints;

	bool mUseSerial = false;
	SerialRef mCom;

	gl::FboRef mFbo;
	
	cv::Mat mContourMat;
	cv::Mat mOutContoursMat;

	vector<vector<cv::Point>> mContours;
};