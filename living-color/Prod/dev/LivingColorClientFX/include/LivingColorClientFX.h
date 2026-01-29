#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class FXImageGroup {
public:
	Surface8uRef mSrcImage;
	cv::Mat mGrayMat;
	cv::Mat mContoursOutMat;
	vector<vector<cv::Point>> mContours;

	FXImageGroup() {}
	FXImageGroup(const string& assetPath);

	bool mIsInitialized = false;
	
	void init();
	void drawImage()
	{
		gl::color(Color(1, 1, 1));
		gl::draw(gl::Texture::create((*mSrcImage)));
	}

	void drawContours()
	{
		gl::color(Color(1, 1, 1));
		gl::draw(gl::Texture::create(fromOcv(mContoursOutMat)));
	}

	void drawGray()
	{
		gl::color(Color(1, 1, 1));
		gl::draw(gl::Texture::create(fromOcv(mGrayMat)));
	}
};

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
	void show(bool isInside, int ledRadius) const {
		Color col = isInside ? mColor : mColor * 0.25;
		gl::color(col);
		gl::drawSolidCircle(mPos, ledRadius);
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
	void setupImageGroups();
	void setupLeds();

	void drawLeds();

	vector<FXImageGroup> mFXImageGroups;
	vector<FXLed> mLeds;

	int mCurrentImageGroupIndex = 0;
};