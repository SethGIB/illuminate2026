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
	gl::Texture2dRef mGrayTex;
	gl::Texture2dRef mContoursTex;

};

const int kWidth = 360;
const int kHeight = 640;
void RsCvTestApp::setup()
{
	mDepthTex = gl::Texture2d::create(kWidth, kHeight);
	mGrayTex = gl::Texture2d::create(kWidth, kHeight);
	mContoursTex = gl::Texture2d::create(kWidth, kHeight);

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
	gl::draw(mGrayTex);
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
	
	mDepthTex->update(depth_frame.get_data(), GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	cv::Mat depthMat(toOcv(mDepthTex->createSource()));
	cv::Mat depthGrayMat(kHeight, kWidth, CV_8UC1);
	cv::Mat threshMat(kHeight, kWidth, CV_8UC1);

	int dataSize = depth_frame.get_data_size();
	int step = dataSize / (kHeight);

	cv::Mat contourMat(kHeight, kWidth, CV_8UC4);
	contourMat.setTo(cv::Scalar(0, 0, 0, 0));
	vector<vector<cv::Point>> contours;

	cv::cvtColor(depthMat, depthGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(depthGrayMat, threshMat, 50, 255, cv::THRESH_BINARY);
	cv::findContours(threshMat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(contourMat, contours, -1, cv::Scalar(0, 0, 255 , 192), 2);
	mGrayTex = gl::Texture2d::create(fromOcv(contourMat));
}

static void prepareSettings( App::Settings *settings )
{
	settings->setWindowSize(kWidth, kHeight);
	settings->setFrameRate(30.0f);
}

CINDER_APP( RsCvTestApp, RendererGl, *prepareSettings )
