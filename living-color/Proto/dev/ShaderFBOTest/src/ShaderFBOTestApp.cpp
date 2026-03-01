#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ShaderFBOTestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ShaderFBOTestApp::setup()
{
}

void ShaderFBOTestApp::mouseDown( MouseEvent event )
{
}

void ShaderFBOTestApp::update()
{
}

void ShaderFBOTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( ShaderFBOTestApp, RendererGl )
