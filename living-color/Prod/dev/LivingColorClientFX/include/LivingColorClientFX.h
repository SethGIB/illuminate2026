#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"
#include "CinderOpenCV.h"
#include "librealsense2/rs.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class FXLed
{
public:
	FXLed() : mPos(vec2()), mId(-1) {}
	FXLed(vec2 _pos, int _id) : mPos(_pos), mId(_id) {}
	FXLed(float _x, float _y, Color _col, int _id) : mPos(vec2(_x, _y)), mColor(_col), mId(_id) {}
	FXLed(float _x, float _y, int _rx, int _ry, Color _col, int _id) : mPos(vec2(_x, _y)), mColor(_col), mRads(vec2(_rx,_ry)), mId(_id) {}
	int getId() const { return mId; }
	vec2 getPos() const { return mPos; }
	vec3 getColor() const { return mColor; }
	vec3 setColor(const vec3& color) { mColor = Color(color); }
	void step(bool isActive) {
		mActive = isActive;
	}
	void show(bool isInside) const {
		Color col = isInside ? mColor : mColor * 0.25;
		gl::color(col);
		gl::drawSolidEllipse(mPos, mRads.x, mRads.y);
	}

	string getJsonString() const {
		Color8u c8u = Color8u(ColorModel::CM_RGB, mColor);
		return "{\"id\": " + to_string(mId) + \
			", \"color\": [" + to_string(c8u.r) + \
			", " + to_string(c8u.g) + \
			", " + to_string(c8u.b) + "]}";
	}

private:
	vec2 mPos;
	vec2 mRads;
	Color mColor;
	int mId;
	bool mActive = false;
};

enum class DrawMode
{
	MAIN,
	DEBUG_COLOR,
	DEBUG_GRAY,
	DEBUG_BINARY,
	DEBUG_CONTOURS
};

class LivingColorClientFXApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	void setupRs();
	void setupLeds();
	void setupImages();
	bool setupCom(const string &port, const int &baud);

	void updateFrames(); //get depth data, post process (rot, thresh), update cv::Mat, scale, find contours
	void updateCom();

	void drawLeds();
	void drawMain();	//default drawing mode

	void debugDrawColor();
	void debugDrawCv(int mode); //draw gray or binary
	void debugDrawContours();

	vector<FXLed> mLeds;
	vector<FXLed> mActiveLeds;

	rs2::pipeline mRs;
	rs2::config mRsConfig;
	rs2::rotation_filter mRsRotFilter;
	rs2::threshold_filter mRsThreshFilter;
	rs2::colorizer mRsColorizer;

	vector<vector<cv::Point>> mContours;
	cv::Mat mDepthMat;
	cv::Mat mGrayMat;
	cv::Mat mBinaryMat;
	cv::Mat mContourMat;

	gl::Texture2dRef mColorTex;
	gl::Texture2dRef mDepthTex;
	gl::Texture2dRef mGrayTex;
	gl::Texture2dRef mBinaryTex;
	gl::Texture2dRef mContoursTex;

	DrawMode mDrawMode = DrawMode::MAIN;

	bool mPortIsOpen = false;
	SerialRef mCom;
};