#include "TagYoureIt.h"

const int kWidth = 1280;
const int kHeight = 720;
const double kSwapTime = 8.0;
const int kNumWalls = 3;

void TagYoureItApp::setup()
{
	mWallTimer = ci::Timer(false);

	setupImagePipeline();
	buildTheWalls();
	//
	mCam.lookAt(vec3(0, 0, 4.8), vec3(0, 0.2, 0));
	mLightPosWorldSpace = vec3(0, 4, 2.88);
	//
	mWallTimer.start();
}

void TagYoureItApp::update()
{
	stepImagePipeline();
	walkTheWalls();
}

void TagYoureItApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatrices(mCam);
	switch (mDrawMode)
	{
	case 0:
		mConcreteWall->setActive(false);
		mMetalWall->setActive(false);
	
		mBrickWall->setActive(true);
		mBrickWall->show();
		break;
	case 1:
		mBrickWall->setActive(false);
		mMetalWall->setActive(false);
		
		mConcreteWall->setActive(true);
		mConcreteWall->show();
		break;
	case 2:
		mBrickWall->setActive(false);
		mConcreteWall->setActive(false);
	
		mMetalWall->setActive(true);
		mMetalWall->show();
		break;
	}
}

void TagYoureItApp::resize()
{
	mCam.setPerspective(60, getWindowAspectRatio(), 1, 1000);
	gl::setMatrices(mCam);
}

void TagYoureItApp::cleanup()
{
	mPipe->stop();
}

void TagYoureItApp::setupImagePipeline()
{
	mDepthColorMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC3);
	mDepthGrayMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mDepthThreshMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC1);
	mContoursMat = cv::Mat::zeros(kHeight, kWidth, CV_8UC4);
	mContours.clear();	
	//
	mRs2Color = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE).loadTopDown(true));
	mDepthAlphaTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RED).dataType(GL_UNSIGNED_BYTE).loadTopDown(true));
	mContoursTex = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGBA).dataType(GL_UNSIGNED_BYTE).loadTopDown(true));
	//
	mRs2Color->bind(4);
	mDepthAlphaTex->bind(5);
	mContoursTex->bind(6);
	//
	mPipe = std::make_shared<rs2::pipeline>();
	auto cfg = rs2::config();
	cfg.enable_stream(RS2_STREAM_COLOR, kWidth, kHeight, RS2_FORMAT_RGB8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, kWidth, kHeight, RS2_FORMAT_Z16, 30);
	//
	mAlignToColor = std::make_shared<rs2::align>(RS2_STREAM_COLOR);
	mColorizer = std::make_shared<rs2::colorizer>();
	mColorizer->set_option(RS2_OPTION_COLOR_SCHEME, 4);
	//
	//
	mDepthThresholder = std::make_shared<rs2::threshold_filter>();
	mDepthThresholder->set_option(RS2_OPTION_MIN_DISTANCE, 0.25f);
	mDepthThresholder->set_option(RS2_OPTION_MAX_DISTANCE, 2.0f);
	//
	mPipe->start(cfg);

}

void TagYoureItApp::buildTheWalls()
{
	mBrickWall = TYI::WallBatch::create();
	mConcreteWall = TYI::WallBatch::create();
	mMetalWall = TYI::WallBatch::create();

	const string vertShaderPath = "shaders/wall.vert";
	const string fragShaderPath = "shaders/wall.frag";

	vec2 brickWallTexScale = vec2(1.667f, 1.0f);
	mBrickWall->loadShaders(vertShaderPath, fragShaderPath, brickWallTexScale);

	vec2 concreteWallTexScale = vec2(1.667f, 1.0f);
	mConcreteWall->loadShaders(vertShaderPath, fragShaderPath, concreteWallTexScale);

	vec2 metalWallTexScale = vec2(1.667f, 1.0f);
	mMetalWall->loadShaders(vertShaderPath, fragShaderPath, metalWallTexScale);

	mBrickWall->loadTextures("textures/TX_brick_d.jpg", "textures/TX_brick_n.jpg", "textures/TX_brick_arm.jpg", "textures/TX_grad_2.png");
	mConcreteWall->loadTextures("textures/TX_concrete_d.jpg", "textures/TX_concrete_n.jpg", "textures/TX_concrete_arm.jpg", "textures/TX_grad_1.png");
	mMetalWall->loadTextures("textures/TX_metal_d.png", "textures/TX_metal_n.jpg", "textures/TX_metal_arm.jpg", "textures/TX_grad_0.png");

	mBrickWall->setupBatch();
	mConcreteWall->setupBatch();
	mMetalWall->setupBatch();
}

void TagYoureItApp::walkTheWalls()
{
	if (mWallTimer.getSeconds() > kSwapTime)
	{
		mWallTimer.stop();
		mDrawMode = (mDrawMode + 1) % kNumWalls;
		mWallTimer.start();
	}

	auto lx = 4.0f * math<float>::cos(getElapsedSeconds() * 0.375f);
	auto lz = lmap<float>(math<float>::sin(getElapsedSeconds() * 0.088f), -1.0, 1.0, 2.0, 4.0);
	auto ly = 3.0f * math<float>::cos(getElapsedSeconds() * 0.65f);
	mLightPosWorldSpace = vec3(lx, ly, lz);

	vec3 lightPosViewSpace = vec3(mCam.getViewMatrix() * vec4(mLightPosWorldSpace, 1));
	switch (mDrawMode)
	{
	case 0:
		mBrickWall->step(lightPosViewSpace);
		break;
	case 1:
		mConcreteWall->step(lightPosViewSpace);
		break;
	case 2:
		mMetalWall->step(lightPosViewSpace);
		break;
	}

}

void TagYoureItApp::stepImagePipeline()
{
	auto frames = mPipe->wait_for_frames();
	frames = mAlignToColor->process(frames);
	auto colorFrame = frames.get_color_frame();
	auto depthFrame = frames.get_depth_frame().apply_filter(*mDepthThresholder).apply_filter(*mColorizer);
	//
	mRs2Color->update(colorFrame.get_data(), GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	//
	mDepthColorMat.data = (uchar*)depthFrame.get_data();
	cv::cvtColor(mDepthColorMat, mDepthGrayMat, cv::COLOR_RGB2GRAY);
	cv::threshold(mDepthGrayMat, mDepthThreshMat, 4, 255, cv::THRESH_BINARY);
	mDepthAlphaTex->update(mDepthThreshMat.data, GL_RED, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	//
	mContoursMat.setTo(cv::Scalar(0, 0, 0, 0));
	cv::findContours(mDepthThreshMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(mContoursMat, mContours, -1, cv::Scalar(48, 48, 48, 255), 18);
	mContoursTex->update(mContoursMat.data, GL_RGBA, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
}

static void prepareSettings( TagYoureItApp::Settings *settings )
{
	settings->setWindowSize( kWidth, kHeight );
	settings->setFrameRate(30);
}

CINDER_APP( TagYoureItApp, RendererGl, *prepareSettings )
