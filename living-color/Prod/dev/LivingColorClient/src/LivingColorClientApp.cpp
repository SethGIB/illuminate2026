#include "LivingColorClient.h"

const int kWindowWidth = 500;
const int kWindowHeight = 800;
const int kGridX = 10;
const int kGridY = 16;
const int kPad = 60;
const int kRadius = 100;

const string kPortName = "COM7";

LCLed::LCLed() : mPos(vec2()), mId(-1) {}

LCLed::LCLed(vec2 _pos, int _id) : mPos(_pos), mId(_id) {}

LCLed::LCLed(float _x, float _y, int _id) : mPos(vec2(_x, _y)), mId(_id) {}

void LCLed::show(bool isInside)
{
	if (isInside)
	{
		gl::drawSolidCircle(mPos, 8);
	}
	else
	{
		gl::drawStrokedCircle(mPos, 4);
	}
}

int LCLed::getId() const
{
	return mId;
}

vec2 LCLed::getPos() const
{
	return mPos;
}

void LivingColorClientApp::setup()
{
	setupCom(kPortName);
	setupLEDs();

	gl::Fbo::Format fboFormat;
	fboFormat.setColorTextureFormat(gl::Texture2d::Format().internalFormat(GL_RGBA8));
	mFbo = gl::Fbo::create(getWindowWidth(), getWindowHeight(), fboFormat);

	mContourMat = cv::Mat::zeros(getWindowHeight(), getWindowWidth(), CV_8UC4);	
	mOutContoursMat = cv::Mat::zeros(getWindowHeight(), getWindowWidth(), CV_8UC4);
}

void LivingColorClientApp::mouseDown( MouseEvent event )
{
}

void LivingColorClientApp::update()
{
	mSelected.clear();
	mTestPoints.clear();
	mMousePos = getWindow()->getMousePos();
	
	updateImageBuffers();
	updateLeds();
}

void LivingColorClientApp::draw()
{
	gl::enableDepth(false);
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatricesWindow(getWindowSize());
	gl::color(Color::white());
	drawLEDs();

	gl::enableAlphaBlending();
	gl::draw(mFbo->getColorTexture());
	
	auto txContours = gl::Texture2d::create(fromOcv(mOutContoursMat));
	gl::draw(txContours);
	gl::disableAlphaBlending();
}

void LivingColorClientApp::setupCom(const string& name)
{
	try
	{
		auto com = Serial::Device(name); //findDevice* and getDevice don't enumerate properly on some systems
		mCom = Serial::create(com, 115200);
		mUseSerial = true;
	}
	catch (SerialExc& e)
	{
		mUseSerial = false;
		console() << "Serial Coms error: " << e.what() << std::endl;
		console() << "Starting in visualization only mode." << std::endl;
	}
}

void LivingColorClientApp::setupLEDs()
{
	const int w = getWindowWidth() - kPad;
	const int h = getWindowHeight() - kPad;
	int count = 0;
	for (int y = 0; y < kGridY; y++)
	{
		float y0 = lmap<float>(y + 0.5f, 0, kGridY, kPad, h);
		for (int x = 0; x < kGridX; x++)
		{
			float x0 = -1;
			if (y % 2 == 0)
			{
				x0 = lmap<float>(x + 0.5f, 0, kGridX, kPad, w);
			}
			else
			{
				x0 = lmap<float>(kGridX - (x + 0.5f), 0, kGridX, kPad, w);
			}
			mLeds.push_back(LCLed(x0, y0, count));
			count+=1;
		}
	}
}

void LivingColorClientApp::updateImageBuffers()
{
	mFbo->bindFramebuffer();
	gl::clear(ColorA(0, 0, 0, 0));
	gl::color(ColorA(vec4(0.0f, 1.0f, 0.0f, 0.5f)));
	gl::drawSolidCircle(mMousePos, kRadius);
	mFbo->unbindFramebuffer();

	auto imgSrc = mFbo->getColorTexture()->createSource();

	cv::Mat tmpMat = toOcv(imgSrc);
	mOutContoursMat.setTo(cv::Scalar(0, 0, 0, 0));
	cv::cvtColor(tmpMat, mContourMat, cv::COLOR_RGBA2GRAY);
	cv::findContours(mContourMat, mContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::drawContours(mOutContoursMat, mContours, -1, cv::Scalar(0, 255, 255, 255), 1);
}

void LivingColorClientApp::updateLeds()
{
	if(mContourMat.empty() || mContours.size() == 0)
		return;

	for (const auto& bound : mContours)
	{
		for (auto& l : mLeds)
		{
			auto p = l.getPos();
			cv::Point testPt(p.x, p.y);
			auto testRes = cv::pointPolygonTest(bound, testPt, false);
			if (testRes >= 0)
			{
				mSelected.push_back(l.getId());
			}
		}
	}

	if (mSelected.size() > 0)
	{
		// remove uniques
		sort(mSelected.begin(), mSelected.end());
		auto mIt = unique(mSelected.begin(), mSelected.end());
		mSelected.erase(mIt, mSelected.end());

		string comMsg = to_string(mSelected[0]);
		for (int s = 1; s < mSelected.size(); s++)
		{
			comMsg += ",";
			comMsg += to_string(mSelected[s]);
		}
		comMsg += "\n"; //termination

		if (mUseSerial)
		{
			mCom->writeString(comMsg);
			mCom->flush();
		}

		console() << comMsg;
	}
}

void LivingColorClientApp::drawLEDs()
{
	for (auto& l : mLeds)
	{
		int i = l.getId();
		int found = std::count(mSelected.begin(), mSelected.end(), i);
		l.show(found > 0);
	}
}

void prepareSettings(App::Settings* settings)
{
	settings->setWindowSize(kWindowWidth, kWindowHeight);
	//settings->setConsoleWindowEnabled(true);
}

CINDER_APP( LivingColorClientApp, RendererGl, *prepareSettings )
