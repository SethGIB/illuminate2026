#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
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

	int getId() const { return mId; }
	vec2 getPos() const { return mPos; }
	vec3 getColor() const { return mColor; }
	vec3 setCOlor(const vec3& color) { mColor = Color(color); }
	void show(bool isInside, int radX, int radY) const {
		Color col = isInside ? mColor : mColor * 0.25;
		gl::color(col);
		gl::drawSolidEllipse(mPos, radX, radY);
	}

private:
	vec2 mPos;
	Color mColor;
	int mId;
};

class LivingColorClientFXApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

private:
	void setupRs();
	void setupLeds();
	void setupImages();

	void updateFrames(); //get depth data, post process (rot, thresh), update cv::Mat, scale, find contours
	\
	void drawLeds();

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

	gl::Texture2dRef mDepthTex;
	gl::Texture2dRef mContoursTex;


};