#include "LivingColorClientFX.h"

const int kWidth = 360;
const int kHeight = 640;
const int kWindowWidth = 360;
const int kWindowHeight = 640;

const float kFPS = 30.0f;
const int kNumLedsX = 18;
const int kNumLedsY = 32;
const float kLedRadiusX = kWindowWidth / (float)(kNumLedsX * 2);
const float kLedRadiusY = kWindowHeight / (float)(kNumLedsY * 2);

const int kSwitchGroupFrames = 45;

const double kDemoWaitTime = 5.0;
const double kDemoRunTime = 6.0;

void LivingColorClientFXApp::setup()
{
	setupImages();
	setupLeds();
	setupRs();

	mDemoTimer = Timer(false);
	mDemoTimer.start(0);
}

void LivingColorClientFXApp::mouseDown( MouseEvent event )
{
	if (mIsRunning == false)
	{
		mIsRunning = true;
		mDemoTimer.start();
	}
}

void LivingColorClientFXApp::update()
{
	updateFrames();
	if(mIsRunning)
		updateDemo();
}

void LivingColorClientFXApp::draw()
{
	gl::clear(Color(0, 0, 0));
	gl::setMatricesWindow(getWindowSize());
	if(mIsRunning)
	{ 
		switch (mDemoMode)
		{
		case 0:
			break;
		case 1:
			demoDrawColor();
			break;
		case 2:
			demoDrawContours(true);
			break;
		case 3:
			demoDrawFull();
			break;
		}
	}
}

void LivingColorClientFXApp::cleanup()
{
	mRs.stop();
}

void LivingColorClientFXApp::setupRs()
{
	mRsConfig.enable_stream(RS2_STREAM_DEPTH, 640, 360, RS2_FORMAT_Z16, 30);
	mRsConfig.enable_stream(RS2_STREAM_COLOR, 640, 360, RS2_FORMAT_RGB8, 30);
	vector<rs2_stream> streams = { RS2_STREAM_COLOR, RS2_STREAM_DEPTH };
	mRsRotFilter = rs2::rotation_filter(streams);
	mRsRotFilter.set_option(RS2_OPTION_ROTATION, -90.0f);

	mRsThreshFilter = rs2::threshold_filter(0.5f, 1.5f);

	mRsColorizer = rs2::colorizer(4);
	mRs.start(mRsConfig);
}

void LivingColorClientFXApp::setupLeds()
{
	int count = 0;
	for (int y = 0; y < kNumLedsY; y++)
	{
		float y0 = lmap<float>(y + 0.5f, 0, kNumLedsY, 0, kWindowHeight);
		for (int x = 0; x < kNumLedsX; x++)
		{
			float x0 = -1;
			if (y % 2 == 0)
			{
				x0 = lmap<float>(x + 0.5f, 0, kNumLedsX, 0, kWindowWidth);
			}
			else
			{
				x0 = lmap<float>(kNumLedsX - (x + 0.5f), 0, kNumLedsX, 0, kWindowWidth);
			}

			auto ledColor = Colorf(ColorModel::CM_HSV, lmap<float>(count, 0, kNumLedsX * kNumLedsY, 0.0f, 1.0f), 1.0f, 1.0f);
			mLeds.push_back(FXLed(x0, y0, ledColor, count));
			count += 1;
		}
	}
}

void LivingColorClientFXApp::setupImages()
{
	//Color for demo
	mDemoColorTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE));
	//
	mDepthTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE));
	mContoursTex = gl::Texture2d::create(kWidth, kHeight);

	mDepthMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC3);
	mGrayMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mBinaryMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mContourMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC4);
}

void LivingColorClientFXApp::updateFrames()
{
	auto frames = mRs.wait_for_frames();
	auto color_frame = frames.get_color_frame().apply_filter(mRsRotFilter);
	mDemoColorTex->update((uchar*)color_frame.get_data(), GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);

	auto depth_frame = frames.get_depth_frame().apply_filter(mRsRotFilter).apply_filter(mRsThreshFilter).apply_filter(mRsColorizer);

	mDepthMat.data = (uchar*)depth_frame.get_data();
	mDepthTex->update(mDepthMat.data, GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);

	mContours.clear();
	mContourMat.setTo(cv::Scalar(0, 0, 0, 0));

	cv::cvtColor(mDepthMat, mGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(mGrayMat, mBinaryMat, 50, 255, cv::THRESH_BINARY);

	cv::Mat rotatedBinaryMat;
	cv::flip(mBinaryMat, rotatedBinaryMat, 0);
	cv::findContours(rotatedBinaryMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	cv::drawContours(mContourMat, mContours, -1, cv::Scalar(255, 64, 0, 255), 4);

	mContoursTex->update(mContourMat.data, GL_RGBA, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
}

void LivingColorClientFXApp::drawLeds()
{
	for (const FXLed& led : mLeds)
	{
		bool isInsideAnyContour = false;
		for( auto contour : mContours )
		{
			if( cv::pointPolygonTest( contour, cv::Point2f( led.getPos().x, led.getPos().y ), false ) >= 0 )
			{
				isInsideAnyContour = true;
				break;
			}
		}
		led.show(isInsideAnyContour, kLedRadiusX, kLedRadiusY);
	}
}

void LivingColorClientFXApp::demoDrawColor()
{
	gl::color(Color::white());
	gl::draw(mDemoColorTex);
}

void LivingColorClientFXApp::demoDrawContours(bool drawDepth)
{
	gl::color(Color::white());
	if (drawDepth)
	{
		gl::draw(mDepthTex);
	}
	
	gl::enableAlphaBlending(true);
	gl::pushMatrices();
	gl::scale(vec2(1, -1));
	gl::translate(vec2(0, -kWindowHeight));
	gl::draw(mContoursTex);
	gl::popMatrices();
	gl::enableAlphaBlending(false);
}

void LivingColorClientFXApp::demoDrawFull()
{
	drawLeds();
	demoDrawContours(false);
}

void LivingColorClientFXApp::updateDemo()
{
	if (mDemoMode == 0 && mDemoTimer.getSeconds() > kDemoWaitTime)
	{
		mDemoMode = 1;
		mDemoTimer.stop();
		mDemoTimer.start();
	}
	else if (mDemoMode == 1 && mDemoTimer.getSeconds() > kDemoRunTime)
	{
		mDemoMode = 2;
		mDemoTimer.stop();
		mDemoTimer.start();
	}
	else if (mDemoMode == 2 && mDemoTimer.getSeconds() > kDemoRunTime)
	{
		mDemoMode = 3;
	}
}

static void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( kWindowWidth, kWindowHeight );
	settings->setFrameRate( kFPS );
	//settings->setConsoleWindowEnabled( true );
}

CINDER_APP( LivingColorClientFXApp, RendererGl, *prepareSettings )
