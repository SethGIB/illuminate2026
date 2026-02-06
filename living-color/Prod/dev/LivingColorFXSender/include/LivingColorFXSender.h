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
	FXLed() : mPos(vec2()) {}
	FXLed(vec2 _pos) : mPos(_pos) {}
	FXLed(float _x, float _y, Color _col) : mPos(vec2(_x, _y)), mColor(Color8u(_col)) {}
	FXLed(float _x, float _y, int _rx, int _ry, Color _col) : mPos(vec2(_x, _y)), mColor(Color8u(_col)), mRads(vec2(_rx,_ry)) {}
	vec2 getPos() const { return mPos; }
	vec3 getColor() const { return mColor; }
	vec3 setColor(const vec3& color) { mColor = Color8u(color); }

	void show() const {
		Colorf fColor = Colorf(mColor);
		Color col = mActive ? fColor : fColor * 0.25;
		gl::color(col);
		gl::drawSolidEllipse(mPos, mRads.x, mRads.y);
	}

	void activate(bool isActive) { mActive = isActive; }	
	bool isActive() const { return mActive; }
private:
	vec2 mPos;
	vec2 mRads;
	Color8u mColor;
	bool mActive = false;
};

class FXSender {
public:
	FXSender() {}
	void init(const string& port, const int& bps) {
		try {
			mSerial = Serial::create(Serial::Device(port), bps);
			mFxBuffer.resize(FXSender::kFrameSize);
			mPortIsOpen = true;
		}
		catch(SerialExc& e) {
			console() << e.what() << endl;
		}
	}
	void sendFrame(const vector<FXLed>& src) {
		uint8_t counter = 0;
		for (const FXLed &l : src)
		{
			Color8u col = l.isActive() ? Color8u(l.getColor()) : Color8u(l.getColor()) * 0.25;
			mFxBuffer[counter * 3 + 0] = col.r;
			mFxBuffer[counter * 3 + 1] = col.g;
			mFxBuffer[counter * 3 + 2] = col.b;
			counter += 1;
		}

		mSerial->writeBytes(mFxBuffer.data(), FXSender::kFrameSize);
		mSerial->flush();
	}

	void close() {
		if(mPortIsOpen)
		{
			mSerial->flush();
			mSerial.reset();
			mPortIsOpen = false;
		}
	}

	bool isPortOpen() const { return mPortIsOpen; }
	static const int kNumLeds = 240;
	static const int kFrameSize = kNumLeds * 3;

private:
	SerialRef mSerial;
	vector<uint8_t> mFxBuffer;
	bool mPortIsOpen = false;
};

enum class DrawMode
{
	MAIN,
	DEBUG_COLOR,
	DEBUG_GRAY,
	DEBUG_BINARY,
	DEBUG_CONTOURS
};

class LivingColorFXSenderApp : public App {
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

	void updateFrames(); //get depth data, post process (rot, thresh), update cv::Mat, scale, find contours
	void updateLeds(); //check if each led is inside any contour, set color accordingly, send to fx

	void drawLeds();
	void drawMain();	//default drawing mode

	void debugDrawColor();
	void debugDrawCv(int mode); //draw gray or binary
	void debugDrawContours();

	vector<FXLed> mLeds;

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

	FXSender mFxSender;
};