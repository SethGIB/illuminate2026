#include "LivingColorClient.h"

const int kGridX = 5;
const int kGridY = 6;
const int kPad = 80;

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
	setupLEDs();
}

void LivingColorClientApp::mouseDown( MouseEvent event )
{
}

void LivingColorClientApp::update()
{
	mSelected.clear();
	mTestPoints.clear();
	mMousePos = getWindow()->getMousePos();
	
	mTestBounds.x1 = mMousePos.x - 80;
	mTestBounds.y1 = mMousePos.y - 80;
	mTestBounds.x2 = mMousePos.x + 80;
	mTestBounds.y2 = mMousePos.y + 80;

	mTestPoints.push_back(cv::Point(mTestBounds.x1, mTestBounds.y1));
	mTestPoints.push_back(cv::Point(mTestBounds.x2, mTestBounds.y1));
	mTestPoints.push_back(cv::Point(mTestBounds.x2, mTestBounds.y2));
	mTestPoints.push_back(cv::Point(mTestBounds.x1, mTestBounds.y2));

	//Now test points
	for (auto& l : mLeds)
	{
		auto p = l.getPos();
		cv::Point testPt(p.x, p.y);
		auto testRes = cv::pointPolygonTest(cv::Mat(mTestPoints), testPt, false);
		if (testRes > 0)
		{
			mSelected.push_back(l.getId());
		}
	}
}

void LivingColorClientApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::setMatricesWindow(getWindowSize());
	gl::color(Color::white());
	drawLEDs();


	gl::enableAlphaBlending();
	gl::color(ColorA(vec4(0.0f, 1.0f, 0.0f, 0.5f)));
	gl::drawSolidRect(mTestBounds);
	gl::disableAlphaBlending();
	
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
			float x0 = lmap<float>(x + 0.5f, 0, kGridX, kPad, w);
			mLeds.push_back(LCLed(x0, y0, count));
			count++;
		}
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
	settings->setWindowSize(500, 600);
}

CINDER_APP( LivingColorClientApp, RendererGl, *prepareSettings )
