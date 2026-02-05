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

void LivingColorClientFXApp::setup()
{
	mDepthTex = gl::Texture2d::create(kWidth, kHeight);
	mGrayTex = gl::Texture2d::create(kWidth, kHeight);
	mContoursTex = gl::Texture2d::create(kWidth, kHeight);

	setupLeds();
	setupRs();
}

void LivingColorClientFXApp::mouseDown( MouseEvent event )
{
}

void LivingColorClientFXApp::update()
{
	updateFrames();
}

void LivingColorClientFXApp::draw()
{
	/*gl::clear(Color(0, 0, 0));
	drawLeds();
	gl::enableAlphaBlending();
	gl::disableAlphaBlending();*/

	gl::clear(Color(0, 0, 0));
	gl::setMatricesWindow(getWindowSize());
	drawLeds();
	
	gl::color(Color::white());
	//gl::draw(mDepthTex, vec2(0, 0));
	gl::enableAlphaBlending(true);
	gl::draw(mGrayTex);
	gl::enableAlphaBlending(false);

}

void LivingColorClientFXApp::setupRs()
{
	mRsConfig.enable_stream(RS2_STREAM_DEPTH, 640, 360, RS2_FORMAT_Z16, 30);
	vector<rs2_stream> streams = { RS2_STREAM_DEPTH };
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

void LivingColorClientFXApp::updateFrames()
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
	mContours.clear();

	cv::cvtColor(depthMat, depthGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(depthGrayMat, threshMat, 50, 255, cv::THRESH_BINARY);
	cv::findContours(threshMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(contourMat, mContours, -1, cv::Scalar(255, 255, 255, 192), 2);
	mGrayTex = gl::Texture2d::create(fromOcv(contourMat));
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

static void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( kWindowWidth, kWindowHeight );
	settings->setFrameRate( kFPS );
	//settings->setConsoleWindowEnabled( true );
}

CINDER_APP( LivingColorClientFXApp, RendererGl, *prepareSettings )
