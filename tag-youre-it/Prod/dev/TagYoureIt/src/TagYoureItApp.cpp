#include "TagYoureIt.h"

const int kWidth = 720;
const int kHeight = 1280;
const double kSwapTime = 8.0;
const int kNumWalls = 3;

void TagYoureItApp::setup()
{
	mWallTimer = ci::Timer(false);

	//setupImagePipeline();
	buildTheWalls();

	mCam.lookAt(vec3(0, 0, 7.05), vec3(0, 0.2, 0));
	mLightPosWorldSpace = vec3(0, 6, 3.67);

	mWallTimer.start();
}

void TagYoureItApp::update()
{
	///stepImagePipeline();
	if(mWallTimer.getSeconds() > kSwapTime)
	{
		mWallTimer.stop();
		mDrawMode = (mDrawMode + 1) % kNumWalls;
		mWallTimer.start();
	}

	auto lx = 4.33f * math<float>::cos(getElapsedSeconds() * 0.375f);
	auto lz = lmap<float>(math<float>::sin(getElapsedSeconds() * 0.088f), -1.0, 1.0, 3.0, 6.0);
	auto ly = 5.0f * math<float>::cos(getElapsedSeconds() * 0.65f);
	mLightPosWorldSpace = vec3(lx, ly, lz);

	vec3 lightPosViewSpace = vec3(mCam.getViewMatrix() * vec4(mLightPosWorldSpace, 1));
	switch(mDrawMode)
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

void TagYoureItApp::cleanup()
{
	mPipe->stop();
	mRs2Color->unbind(4);
	mRs2Depth->unbind(5);

	mBrickWall->setActive(false);
	mConcreteWall->setActive(false);
	mMetalWall->setActive(false);
}

void TagYoureItApp::setupImagePipeline()
{
	mRs2Color = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE));
	mRs2Depth = gl::Texture2d::create(kWidth, kHeight, gl::Texture2d::Format().internalFormat(GL_RED_INTEGER).dataType(GL_UNSIGNED_SHORT));
	mRs2Color->bind(4);
	mRs2Depth->bind(5);

	mPipe = std::make_shared<rs2::pipeline>();
	auto cfg = rs2::config();
	cfg.enable_stream(RS2_STREAM_COLOR, kHeight, kWidth, RS2_FORMAT_RGB8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, kHeight, kWidth, RS2_FORMAT_Z16, 30);

	mAlignToColor = std::make_shared<rs2::align>(RS2_STREAM_COLOR);
	mColorizer = std::make_shared<rs2::colorizer>();
	mColorizer->set_option(RS2_OPTION_COLOR_SCHEME, 2);

	vector<rs2_stream> streams_to_rotate{ RS2_STREAM_COLOR, RS2_STREAM_DEPTH };
	mImgRotator = std::make_shared<rs2::rotation_filter>(streams_to_rotate);
	mImgRotator->set_option(RS2_OPTION_ROTATION, -90.0f);

	mDepthThresholder = std::make_shared<rs2::threshold_filter>();
	mDepthThresholder->set_option(RS2_OPTION_MIN_DISTANCE, 0.25f);
	mDepthThresholder->set_option(RS2_OPTION_MAX_DISTANCE, 2.0f);

	mPipe->start(cfg);

}

void TagYoureItApp::buildTheWalls()
{
	mBrickWall = TYI::WallBatch::create();
	mConcreteWall = TYI::WallBatch::create();
	mMetalWall = TYI::WallBatch::create();

	const string vertShaderPath = "shaders/wall.vert";
	const string fragShaderPath = "shaders/wall.frag";

	vec2 brickWallTexScale = vec2(1.667f, 1.667f * 3.0f) * 0.667f;
	mBrickWall->loadShaders(vertShaderPath, fragShaderPath, brickWallTexScale);

	vec2 concreteWallTexScale = vec2(0.5625f, 1.667f) * 2.0f;
	mConcreteWall->loadShaders(vertShaderPath, fragShaderPath, concreteWallTexScale);

	vec2 metalWallTexScale = vec2(0.875f, 1.0f);
	mMetalWall->loadShaders(vertShaderPath, fragShaderPath, metalWallTexScale);

	mBrickWall->loadTextures("textures/TX_brick_d.jpg", "textures/TX_brick_n.jpg", "textures/TX_brick_arm.jpg", "textures/TX_grad_0.png");
	mConcreteWall->loadTextures("textures/TX_concrete_d.jpg", "textures/TX_concrete_n.jpg", "textures/TX_concrete_arm.jpg", "textures/TX_grad_1.png");
	mMetalWall->loadTextures("textures/TX_metal_d.png", "textures/TX_metal_n.jpg", "textures/TX_metal_arm.jpg", "textures/TX_grad_2.png");

	mBrickWall->setupBatch();
	mConcreteWall->setupBatch();
	mMetalWall->setupBatch();
}

void TagYoureItApp::stepImagePipeline()
{
	auto frames = mPipe->wait_for_frames();
	//frames = mDepthThresholder->process(frames);
	frames = mAlignToColor->process(frames);
	frames = mImgRotator->process(frames);
	auto colorFrame = frames.get_color_frame();
	auto depthFrame = frames.get_depth_frame().apply_filter(*mDepthThresholder);

	mRs2Color->update(colorFrame.get_data(), GL_RGB, GL_UNSIGNED_BYTE, 0, kWidth, kHeight);
	mRs2Depth->update(depthFrame.get_data(), GL_RED_INTEGER, GL_UNSIGNED_SHORT, 0, kWidth, kHeight);
}

static void prepareSettings( TagYoureItApp::Settings *settings )
{
	settings->setWindowSize( 720, 1280 );
	settings->setFrameRate(30);
}

CINDER_APP( TagYoureItApp, RendererGl, *prepareSettings )
