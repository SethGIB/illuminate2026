#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"
#include "cinder/Serial.h"
#include "cinder/Json.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LCLed
{
public:
	LCLed() : mPos(vec2()), mId(-1) {}
	LCLed(vec2 _pos, int _id) : mPos(_pos), mId(_id) {}
	LCLed(float _x, float _y, Color _col, int _id) : mPos(vec2(_x, _y)), mColor(_col), mId(_id) {}

	void show(bool isInside);
	int getId() const { return mId; }
	vec2 getPos() const { return mPos; }
	vec3 getColor() const { return mColor; }
	vec3 setCOlor(const vec3& color) { mColor = Color(color); }
	string getJsonString() const {

	}

private:
	vec2 mPos;
	Color mColor;
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
	vector<LCLed> mSelected;
	
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