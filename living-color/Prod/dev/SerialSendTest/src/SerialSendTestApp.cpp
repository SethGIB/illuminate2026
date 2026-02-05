#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Serial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const string kPortName = "COM7";
const int kBaud = 115200;

const float kFPS = 30.0f;

class SerialSendTestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	void cleanup() override;

private:
	bool setupCom(const string &port, const int &bps);

	bool mPortOpen = false;
	SerialRef mCom;
};

void SerialSendTestApp::setup()
{
	mPortOpen = setupCom(kPortName, kBaud);
	if (mPortOpen)
	{
		console() << "Opened Port for send: " << mCom->getDevice().getName() << endl;
	}
}

void SerialSendTestApp::mouseDown( MouseEvent event )
{
}

void SerialSendTestApp::update()
{
	if (mPortOpen)
	{
		auto data = to_string(getElapsedFrames()) + "\n";
		//mCom->
		mCom->writeString(data);
		console() << "Wrote: " << data;
		mCom->flush();
	}
}

void SerialSendTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

void SerialSendTestApp::cleanup()
{
	if (mPortOpen)
	{
		mCom->flush();
	}
}

bool SerialSendTestApp::setupCom(const string &port, const int &bps)
{
	bool retval = false;
	try
	{
		auto port = Serial::Device(kPortName);
		mCom = Serial::create(port, kBaud);
		retval = true;
	}
	catch (SerialExc& e)
	{
		console() << e.what() << endl;
	}

	return retval;
}

void prepareSettings(App::Settings* settings)
{
	settings->setFrameRate(kFPS);
	settings->setConsoleWindowEnabled(true);
}

CINDER_APP( SerialSendTestApp, RendererGl, *prepareSettings )
