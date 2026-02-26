#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "librealsense2/rs.hpp"
#include "WallBatch.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef std::shared_ptr<rs2::align> Rs2AlignerRef;
typedef std::shared_ptr<rs2::colorizer> Rs2ColorizerRef;
typedef std::shared_ptr<rs2::pipeline> Rs2PipelineRef;
typedef std::shared_ptr<rs2::rotation_filter> Rs2ImgRotatorRef;
typedef std::shared_ptr<rs2::threshold_filter> Rs2ThresholderRef;

class TagYoureItApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	void setupImagePipeline();
	void buildTheWalls();

	void stepImagePipeline();
	Rs2PipelineRef mPipe;
	Rs2AlignerRef mAlignToColor;
	Rs2ColorizerRef mColorizer;
	Rs2ImgRotatorRef mImgRotator;
	Rs2ThresholderRef mDepthThresholder;

	gl::Texture2dRef mRs2Color;
	gl::Texture2dRef mRs2Depth;

	TYI::WallBatchRef mBrickWall;
	TYI::WallBatchRef mConcreteWall;
	TYI::WallBatchRef mMetalWall;

	int mDrawMode = 0;
	ci::Timer mWallTimer;

	CameraPersp	mCam;
	vec3 mLightPosWorldSpace;
};