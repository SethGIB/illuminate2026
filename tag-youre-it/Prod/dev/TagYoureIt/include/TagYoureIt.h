#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "CinderOpenCV.h"
#include "librealsense2/rs.hpp"
#include "WallBatch.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef std::shared_ptr<rs2::align> Rs2AlignerRef;
typedef std::shared_ptr<rs2::colorizer> Rs2ColorizerRef;
typedef std::shared_ptr<rs2::pipeline> Rs2PipelineRef;
typedef std::shared_ptr<rs2::threshold_filter> Rs2ThresholderRef;

class TagYoureItApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void resize() override;
	void cleanup() override;

private:
	void setupImagePipeline();
	void buildTheWalls();
	void walkTheWalls();

	void stepImagePipeline();
	Rs2PipelineRef mPipe;
	Rs2AlignerRef mAlignToColor;
	Rs2ColorizerRef mColorizer;
	Rs2ThresholderRef mDepthThresholder;

	gl::Texture2dRef mRs2Color;
	gl::Texture2dRef mDepthAlphaTex;
	cv::Mat mDepthThreshMat;
	
	cv::Mat mDepthColorMat;
	cv::Mat mDepthGrayMat;
	
	cv::Mat mContoursMat;
	gl::Texture2dRef mContoursTex;
	vector<vector<cv::Point>> mContours;

	TYI::WallBatchRef mBrickWall;
	TYI::WallBatchRef mConcreteWall;
	TYI::WallBatchRef mMetalWall;

	int mDrawMode = 0;
	ci::Timer mWallTimer;

	CameraPersp	mCam;
	vec3 mLightPosWorldSpace;
};