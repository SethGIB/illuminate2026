#include "LivingColorClientFX.h"

const int kWidth = 360;
const int kHeight = 640;
const int kWindowWidth = 360;
const int kWindowHeight = 640;

const float kFPS = 30.0f;
const int kNumLedsX = 18;
const int kNumLedsY = 32;
const int kLedRadiusX = kWindowWidth / (float)(kNumLedsX * 2);
const int kLedRadiusY = kWindowHeight / (float)(kNumLedsY * 2);

const string kPortName = "COM7"; //"COM3" for windows, "/dev/tty.usbmodem14101" for mac
const int kBaudRate = 115200;

void LivingColorClientFXApp::setup()
{
	setupImages();
	setupLeds();
	setupRs();

	mPortIsOpen = setupCom(kPortName, kBaudRate);
}

void LivingColorClientFXApp::mouseDown( MouseEvent event )
{
}

void LivingColorClientFXApp::keyDown(KeyEvent event)
{
	if (event.getChar() == '0')
	{
		mDrawMode = DrawMode::MAIN;
	}
	else if (event.getChar() == '1')
	{
		mDrawMode = DrawMode::DEBUG_COLOR;
	}
	else if (event.getChar() == '2')
	{
		mDrawMode = DrawMode::DEBUG_GRAY;
	}
	else if(event.getChar() == '3')
	{
		mDrawMode = DrawMode::DEBUG_BINARY;
	}
	else if (event.getChar() == '4')
	{
		mDrawMode = DrawMode::DEBUG_CONTOURS;
	}
}

void LivingColorClientFXApp::update()
{
	updateFrames();
	if(mPortIsOpen)
	{
		updateCom();
	}
}

void LivingColorClientFXApp::draw()
{
	gl::clear(Color::black());
	gl::setMatricesWindow(getWindowSize());

	switch (mDrawMode)
	{
		case DrawMode::MAIN:
			drawMain();
			break;
		case DrawMode::DEBUG_COLOR:
			debugDrawColor();
			break;
		case DrawMode::DEBUG_GRAY:
			debugDrawCv(0);
			break;
		case DrawMode::DEBUG_BINARY:
			debugDrawCv(1);
			break;
		case DrawMode::DEBUG_CONTOURS:
			debugDrawContours();
			break;
	}	
}

void LivingColorClientFXApp::cleanup()
{
	mRs.stop();
	if(mPortIsOpen)
	{
		mCom->flush();
	}
}

void LivingColorClientFXApp::setupRs()
{
	mRsConfig.enable_stream(RS2_STREAM_COLOR, 640, 360, RS2_FORMAT_RGB8, 30);
	mRsConfig.enable_stream(RS2_STREAM_DEPTH, 640, 360, RS2_FORMAT_Z16, 30);
	vector<rs2_stream> streams = { RS2_STREAM_DEPTH, RS2_STREAM_COLOR };
	mRsRotFilter = rs2::rotation_filter(streams);
	mRsRotFilter.set_option(RS2_OPTION_ROTATION, -90.0f);

	mRsThreshFilter = rs2::threshold_filter(0.2f, 1.5f);

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
			mLeds.push_back(FXLed(x0, y0, kLedRadiusX, kLedRadiusY, ledColor, count));
			count += 1;
		}
	}
}

void LivingColorClientFXApp::setupImages()
{
	mColorTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE));
	mDepthTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE));
	mGrayTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RED).dataType(GL_UNSIGNED_BYTE));
	mBinaryTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RED).dataType(GL_UNSIGNED_BYTE));
	mContoursTex = gl::Texture2d::create(kWidth, kHeight);

	mDepthMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC3);
	mGrayMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mBinaryMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mContourMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC4);
}

bool LivingColorClientFXApp::setupCom(const string &port, const int &baud)
{
	bool ret = false;
	try
	{
		auto comPort = Serial::Device(port);
		mCom = Serial::create(comPort, baud);
		ret = true;
	}
	catch (SerialExc& e)
	{
		console() << e.what() << endl;
	}

	return ret;
}

void LivingColorClientFXApp::updateFrames()
{
	auto frames = mRs.wait_for_frames();
	if (mDrawMode == DrawMode::DEBUG_COLOR)
	{
		auto color_frame = frames.get_color_frame().apply_filter(mRsRotFilter);
		mColorTex->update(color_frame.get_data(), GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	}

	auto depth_frame = frames.get_depth_frame().apply_filter(mRsRotFilter).apply_filter(mRsThreshFilter).apply_filter(mRsColorizer);
	mDepthMat.data = (uchar*)depth_frame.get_data();
	mDepthTex->update(mDepthMat.data, GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);

	mContours.clear();
	mContourMat.setTo(cv::Scalar(0, 0, 0, 0));

	cv::cvtColor(mDepthMat, mGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(mGrayMat, mBinaryMat, 4, 255, cv::THRESH_BINARY);

	//rotate the binary mat
	cv::Mat rotatedBinaryMat;
	cv::flip(mBinaryMat, rotatedBinaryMat, 0); // flip around x-axis (vertical flip)
	cv::findContours(rotatedBinaryMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	cv::drawContours(mContourMat, mContours, -1, cv::Scalar(255, 64, 0, 255), 4);

	mContoursTex->update(mContourMat.data, GL_RGBA, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	if(mDrawMode == DrawMode::DEBUG_GRAY)
	{
		mGrayTex->update(mGrayMat.data, GL_RED, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	}
	if(mDrawMode == DrawMode::DEBUG_BINARY)
	{
		mBinaryTex->update(mBinaryMat.data, GL_RED, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	}
}

void LivingColorClientFXApp::updateCom()
{
	auto data = to_string(mContours.size()) + "\n"; //replace with json LED map
	mCom->writeString(data);
	mCom->flush();
}

void LivingColorClientFXApp::drawLeds()
{
	mActiveLeds.clear();
	for (const FXLed& led : mLeds)
	{
		bool isInsideAnyContour = false;
		for( auto contour : mContours )
		{
			if( cv::pointPolygonTest( contour, cv::Point2f( led.getPos().x, led.getPos().y ), false ) >= 0 )
			{
				isInsideAnyContour = true;
				mActiveLeds.push_back(led);
				break;
			}
		}
		led.show(isInsideAnyContour);
	}

}

void LivingColorClientFXApp::drawMain()
{
	drawLeds();

	gl::color(Color::white());
	gl::enableAlphaBlending(true);
	gl::pushMatrices();
	gl::scale(vec2(1, -1));
	gl::translate(vec2(0, -kWindowHeight));
	gl::draw(mContoursTex);
	gl::popMatrices();
	gl::enableAlphaBlending(false);

}

void LivingColorClientFXApp::debugDrawColor()
{
	gl::color(Color::white());
	gl::draw(mColorTex);
}

void LivingColorClientFXApp::debugDrawCv(int mode)
{
	gl::color(Color::white());
	switch (mode)
	{
		case 0:
			gl::draw(mGrayTex);
			break;
		case 1:
			gl::draw(mBinaryTex);
			break;
	}
}

void LivingColorClientFXApp::debugDrawContours()
{
	gl::color(Color::white());
	gl::draw(mDepthTex);
	gl::enableAlphaBlending(true);
	gl::pushMatrices();
	gl::scale(vec2(1, -1));
	gl::translate(vec2(0, -kWindowHeight));
	gl::draw(mContoursTex);
	gl::popMatrices();
	gl::enableAlphaBlending(false);

}

static void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( kWindowWidth, kWindowHeight );
	settings->setFrameRate( kFPS );
	//settings->setConsoleWindowEnabled( true );
}

CINDER_APP( LivingColorClientFXApp, RendererGl, *prepareSettings )
