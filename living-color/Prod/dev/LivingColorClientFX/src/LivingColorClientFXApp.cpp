#include "LivingColorClientFX.h"

FXImageGroup::FXImageGroup(const string& assetPath)
{
	ImageSource::Options options;
	//options
	mSrcImage = Surface8u::create(loadImage(loadAsset(assetPath), options, "png"));
	mContoursOutMat = cv::Mat::zeros(mSrcImage->getHeight(), mSrcImage->getWidth(), CV_8UC4);
	mGrayMat = cv::Mat::zeros(mSrcImage->getHeight(), mSrcImage->getWidth(), CV_8UC1);
}

void FXImageGroup::init()
{
	try {
		cv::Mat tempMat = toOcv(*mSrcImage);
		cv::cvtColor(tempMat, mGrayMat, cv::COLOR_RGB2GRAY);
		cv::findContours(mGrayMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
		cv::drawContours(mContoursOutMat, mContours, -1, cv::Scalar(255, 255, 255, 128), 2);
		mIsInitialized = true;
	}
	catch (cv::Exception& e) {
		console() << "Error: " << e.what() << std::endl;
	}
}

const int kWindowWidth = 500;
const int kWindowHeight = 800;
const float kFPS = 30.0f;
const int kNumLedsX = 20;
const int kNumLedsY = 30;
const float kLedRadius = kWindowWidth / (kNumLedsX * 2.0f);

const int kSwitchGroupFrames = 45;

void LivingColorClientFXApp::setup()
{
	setupImageGroups();
	setupLeds();
}

void LivingColorClientFXApp::mouseDown( MouseEvent event )
{
}

void LivingColorClientFXApp::update()
{
	if(getElapsedFrames() % kSwitchGroupFrames == 0 && !mFXImageGroups.empty())
	{
		mCurrentImageGroupIndex = (mCurrentImageGroupIndex + 1) % mFXImageGroups.size();
	}
}

void LivingColorClientFXApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	drawLeds();
	gl::enableAlphaBlending();
	mFXImageGroups[mCurrentImageGroupIndex].drawContours();
	gl::disableAlphaBlending();

}

void LivingColorClientFXApp::setupImageGroups()
{
	const auto& assetDirs = getAssetDirectories();
	if (assetDirs.size() == 1)
	{
		const auto& assetDir = assetDirs[0];
		if (filesystem::exists(assetDir))
		{
			for (const auto& entry : filesystem::directory_iterator(assetDir))
			{
				if (entry.is_regular_file())
				{
					const auto& path = entry.path();
					if (path.extension() == ".png")
					{
						FXImageGroup fxImageGroup(path.filename().string());
						fxImageGroup.init();
						if (fxImageGroup.mIsInitialized)
						{
							mFXImageGroups.push_back(fxImageGroup);
							console() << "Loaded and initialized image: " << path.filename().string() << std::endl;
						}
						else
						{
							console() << "Failed to initialize image: " << path.filename().string() << std::endl;
						}
					}
				}
			}
		}
		else
		{
			console() << "Asset directory does not exist: " << assetDir.string() << std::endl;
		}
	}
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

void LivingColorClientFXApp::drawLeds()
{
	for (const FXLed& led : mLeds)
	{
		bool isInsideAnyContour = false;
		for (const auto& contour : mFXImageGroups[mCurrentImageGroupIndex].mContours)
		{
			if (cv::pointPolygonTest(contour, cv::Point2f(led.getPos().x, led.getPos().y), false) >= 0)
			{
				isInsideAnyContour = true;
				break;
			}
		}
		led.show(isInsideAnyContour, kLedRadius);
	}

}

static void prepareSettings( App::Settings* settings )
{
	settings->setWindowSize( kWindowWidth, kWindowHeight );
	settings->setFrameRate( kFPS );
	//settings->setConsoleWindowEnabled( true );
}

CINDER_APP( LivingColorClientFXApp, RendererGl, *prepareSettings )
