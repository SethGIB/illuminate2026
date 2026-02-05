#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderOpenCV.h"
#include "librealsense2/rs.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class RsCvTestApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void quit() override;

private:
	void updateFrames(); //get depth data, post process (rot, thresh), update cv::Mat, scale, find contours

	rs2::pipeline mRs;
	rs2::config mRsConfig;

	rs2::rotation_filter mRsRotFilter;
	rs2::threshold_filter mRsThreshFilter;

	rs2::colorizer mRsColorizer;

	gl::Texture2dRef mDepthTex;
	gl::Texture2dRef mContoursTex;

	cv::Mat mDepthMat;
	cv::Mat mGrayMat;
	cv::Mat mBinaryMat;
	cv::Mat mContourMat;

	vector<vector<cv::Point>> mContours;

};

const int kWidth = 360;
const int kHeight = 640;
void RsCvTestApp::setup()
{
	auto txFormat = gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE);

	mDepthTex = gl::Texture2d::create(kWidth, kHeight, txFormat);
	mContoursTex = gl::Texture2d::create(kWidth, kHeight);

	mDepthMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC3);
	mGrayMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mBinaryMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mContourMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC4);

	mRsConfig.enable_stream(RS2_STREAM_DEPTH, 640, 360, RS2_FORMAT_Z16, 30);
	vector<rs2_stream> streams = { RS2_STREAM_DEPTH };
	mRsRotFilter = rs2::rotation_filter(streams);
	mRsRotFilter.set_option(RS2_OPTION_ROTATION, -90.0f);
	
	mRsThreshFilter = rs2::threshold_filter(0.5f, 1.5f);
	
	mRsColorizer = rs2::colorizer(4);	
	mRs.start(mRsConfig);
}

void RsCvTestApp::mouseDown( MouseEvent event )
{
}

void RsCvTestApp::update()
{
	updateFrames();
}

void RsCvTestApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::setMatricesWindow(getWindowSize());
	gl::color(Color::white());
	gl::draw(mDepthTex, vec2(0, 0));
	gl::enableAlphaBlending(true);
	gl::draw(mContoursTex, vec2(0, 0));
	gl::enableAlphaBlending(false);
}

void RsCvTestApp::quit()
{
	mRs.stop();
}

void RsCvTestApp::updateFrames()
{
	auto frames = mRs.wait_for_frames();
	auto depth_frame = frames.get_depth_frame().apply_filter(mRsRotFilter).apply_filter(mRsThreshFilter).apply_filter(mRsColorizer);

	mDepthMat.data = (uchar*)depth_frame.get_data();
	mDepthTex->update(mDepthMat.data, GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);

	mContours.clear();
	mContourMat.setTo(cv::Scalar(0,0,0,0));

	cv::cvtColor(mDepthMat, mGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(mGrayMat, mBinaryMat, 50, 255, cv::THRESH_BINARY);
	cv::findContours(mBinaryMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(mContourMat, mContours, -1, cv::Scalar(255, 64, 0 , 255), 4);

	mContoursTex->update(mContourMat.data, GL_RGBA, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
}

static void prepareSettings( App::Settings *settings )
{
	settings->setWindowSize(kWidth, kHeight);
	settings->setFrameRate(30.0f);
}

CINDER_APP( RsCvTestApp, RendererGl, *prepareSettings )
