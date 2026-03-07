#include "LivingColorFXSender.h"

const int kWidth = 360;
const int kHeight = 640;
const int kWindowWidth = 540;
const int kWindowHeight = 960;

const float kFPS = 30.0f;
const int kNumLedsX = 12;
const int kNumLedsY = 18;
const int kLedRadiusX = ceil(kWindowWidth / (float)(kNumLedsX * 2));
const int kLedRadiusY = ceil(kWindowHeight / (float)(kNumLedsY * 2));

const string kIpAddr = "192.168.0.8"; 
const int kPortNum = 50051;

const bool kUseNetwork = false;
const bool kUseRs = true;

const double kPlasmaSwitchInterval = 6.0; //seconds
const double kMinContourArea = 10000.0; // Minimum area for contours to be considered valid
const double kDemoSwitchInterval = 3.26577; //seconds

void LivingColorFXSenderApp::setup()
{
	mDrawMode = DrawMode::MAIN;
	setupLeds();
	setupPlasma();
	mCurrentPlasma = mPlasmaClassic;
	mPlasmaTime = Timer(true);
	mDemoTimer = Timer(true);
	
	if(kUseRs)
	{
		setupRs();
		setupImages();
	}
	
	if(kUseNetwork)
		mFxSender.init(kIpAddr, kPortNum);

	mCurrentPlasma->setActive(true);
}

void LivingColorFXSenderApp::mouseDown( MouseEvent event )
{
}

void LivingColorFXSenderApp::keyDown(KeyEvent event)
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
	else if (event.getChar() == '5')
	{
		mDrawMode = DrawMode::DEBUG_PLASMA;
	}
	else if (event.getChar() == '6')
	{
		mDrawMode = DrawMode::DEBUG_LEDSONLY;
	}
	// Toggle demo mode with 'd'
	else if (event.getChar() == 'd' || event.getChar() == 'D')
	{
		mDemoMode = !mDemoMode;
		demoMode(mDemoMode);
	}
}

void LivingColorFXSenderApp::update()
{
	if (kUseRs)
	{
		updateFrames();
	}
	updateLeds();
	swapPlasma();

	// Change mode every 5 seconds
	if (mDemoMode)
	{
		if (mDemoTimer.getSeconds() >= kDemoSwitchInterval)
		{
			// Cast to int, increment, wrap around using COUNT, and cast back
			int nextMode = (static_cast<int>(mDrawMode) + 1) % static_cast<int>(DrawMode::COUNT);
			mDrawMode = static_cast<DrawMode>(nextMode);

			// Restart the timer
			mDemoTimer.start();
		}
	}
}

void LivingColorFXSenderApp::draw()
{
	gl::clear(Color::black());
	gl::setMatricesWindow(getWindowSize());
	mCurrentPlasma->render(ivec2(kWidth, kHeight), Area(0, 0, kWindowWidth, kWindowHeight));
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
		case DrawMode::DEBUG_PLASMA:
			gl::draw(mCurrentPlasma->getTexture());
			break;
		case DrawMode::DEBUG_LEDSONLY:
			drawLeds();
			break;
	}	
}

void LivingColorFXSenderApp::cleanup()
{
	if (kUseRs)
	{
		mRs.stop();
	}
	if (kUseNetwork)
	{
		mFxSender.close();
	}
}

void LivingColorFXSenderApp::setupRs()
{
	mRsConfig.enable_stream(RS2_STREAM_COLOR, kHeight, kWidth, RS2_FORMAT_RGB8, 30);
	mRsConfig.enable_stream(RS2_STREAM_DEPTH, kHeight, kWidth, RS2_FORMAT_Z16, 30);
	vector<rs2_stream> streams = { RS2_STREAM_DEPTH, RS2_STREAM_COLOR };
	mRsRotFilter = rs2::rotation_filter(streams);
	mRsRotFilter.set_option(RS2_OPTION_ROTATION, -90.0f);

	mRsThreshFilter = rs2::threshold_filter(0.5f, 2.0f);

	mRsColorizer = rs2::colorizer(4);
	mRs.start(mRsConfig);
}

void LivingColorFXSenderApp::setupLeds()
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

			auto ledColor = Color8u(64,64,64);
			mLeds.push_back(FXLed(x0, y0, kLedRadiusX, kLedRadiusY, ledColor));
			count += 1;
		}
	}
}

void LivingColorFXSenderApp::setupImages()
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

void LivingColorFXSenderApp::setupPlasma()
{
	mPlasmaClassic = Plasma::create();
	mPlasmaClassic->init("shaders/passthru.vert", "shaders/plasma_classic.frag", "textures/classic/TX_Noise_Classic.png", "textures/classic/TX_Grad_Classic.png", ivec2(kWidth, kHeight));

	mPlasmaNeo = Plasma::create();
	mPlasmaNeo->init("shaders/passthru.vert", "shaders/plasma_neo.frag", "textures/neo/TX_Noise_Neo.png", "textures/neo/TX_Grad_Neo.png", ivec2(kWidth, kHeight));

	mPlasmaTech = Plasma::create();
	mPlasmaTech->init("shaders/passthru.vert", "shaders/plasma_tech.frag", "textures/tech/TX_Noise_Tech.png", "textures/tech/TX_Grad_Tech.png", ivec2(kWidth, kHeight));
	
	mPlasmaPsychedelic = Plasma::create();
	mPlasmaPsychedelic->init("shaders/passthru.vert", "shaders/plasma_psyche.frag", "textures/psyche/TX_Noise_Psyche.png", "textures/psyche/TX_Grad_Psyche.png", ivec2(kWidth, kHeight));
}

void LivingColorFXSenderApp::updateFrames()
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

	// --- NEW FILTERING CODE ---
	vector<vector<cv::Point>> filteredContours;
	const double minContourArea = kMinContourArea; // Adjust this threshold as needed

	for (const auto& contour : mContours) {
		if (cv::contourArea(contour) >= minContourArea) {
			filteredContours.push_back(contour);
		}
	}
	mContours = filteredContours; // Replace the original list with the filtered one
	// --------------------------

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

void LivingColorFXSenderApp::updateLeds()
{
	auto surf = mCurrentPlasma->getPixels();
	for (FXLed& led : mLeds)
	{
		auto lPos = led.getPos();
		//ivec2 scaledPos = ivec2(lmap<float>(lPos.x, 0, kWindowWidth, 0, kWidth), lmap<float>(lPos.y, 0, kWindowHeight, 0, kHeight));
		//ivec2 scaledPos ()
		auto pixelColor = surf.getPixel(ivec2(lPos));
		led.setColor(Color8u(pixelColor.r, pixelColor.g, pixelColor.b));
		if (kUseRs && mContours.size() > 0)
		{
			led.activate(false);
			bool isInsideAnyContour = false;
			for (auto contour : mContours)
			{
				if (cv::pointPolygonTest(contour, cv::Point2f(led.getPos().x, led.getPos().y), false) >= 0)
				{
					led.activate(true);
					break;
				}
			}
		}
		else
		{
			led.activate(true);
		}
	}
	if (kUseNetwork && mFxSender.isPortOpen())
	{
		mFxSender.sendFrame(mLeds);
	}
}

void LivingColorFXSenderApp::swapPlasma()
{
	if (mPlasmaTime.getSeconds() >= kPlasmaSwitchInterval)
	{
		mCurrentPlasma->setActive(false);
		mPlasmaTime.stop();
		if (mCurrentPlasma == mPlasmaClassic)
		{
			mCurrentPlasma = mPlasmaTech;
		}
		else if (mCurrentPlasma == mPlasmaTech)
		{
			mCurrentPlasma = mPlasmaPsychedelic;
		}
		else if (mCurrentPlasma == mPlasmaPsychedelic)
		{
			mCurrentPlasma = mPlasmaNeo;
		}
		else
		{
			mCurrentPlasma = mPlasmaClassic;
		}
		mCurrentPlasma->setActive(true);
		mPlasmaTime.start();
	}
}

void LivingColorFXSenderApp::drawLeds()
{
	for (const FXLed& led : mLeds)
	{
		led.show();
	}
}

void LivingColorFXSenderApp::drawMain()
{
	drawLeds();

	gl::color(Color::white());
	gl::enableAlphaBlending(true);
	gl::pushMatrices();
	gl::scale(vec2(1, -1));
	gl::translate(vec2(0, -kHeight));
	gl::draw(mContoursTex);
	gl::popMatrices();
	gl::disableAlphaBlending();

}

void LivingColorFXSenderApp::debugDrawColor()
{
	gl::color(Color::white());
	gl::draw(mColorTex);
}

void LivingColorFXSenderApp::debugDrawCv(int mode)
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

void LivingColorFXSenderApp::debugDrawContours()
{
	gl::color(Color::white());
	gl::draw(mDepthTex);
	gl::enableAlphaBlending(true);
	gl::pushMatrices();
	gl::scale(vec2(1, -1));
	gl::translate(vec2(0, -kHeight));
	gl::draw(mContoursTex);
	gl::popMatrices();
	gl::enableAlphaBlending(false);

}

void LivingColorFXSenderApp::demoMode(const bool& enable)
{
	if (enable)
	{
		mDemoTimer.start();
	}
	else
	{
		mDemoTimer.stop();
		mDrawMode = DrawMode::MAIN;
	}
}

static void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( kWindowWidth, kWindowHeight );
	settings->setFrameRate( kFPS );
	//settings->setConsoleWindowEnabled( true );
}

CINDER_APP( LivingColorFXSenderApp, RendererGl, *prepareSettings )