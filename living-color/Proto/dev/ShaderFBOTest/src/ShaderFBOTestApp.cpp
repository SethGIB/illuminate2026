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

private:
	void drawPixelGrid(const int& gridX, const int& gridY, const gl::FboRef& mFbo);

	gl::Texture2dRef mNoiseBase;
	gl::Texture2dRef mNoiseCoarse;
	gl::Texture2dRef mNoiseMid;
	gl::Texture2dRef mNoiseFine;
	gl::Texture2dRef mNoiseGrad;

	gl::FboRef mFbo;
	gl::GlslProgRef mShader;

	Surface8uRef mColorSrc;
};

const int kWidth = 540;
const int kHeight = 960;
const int kGridX = 12;
const int kGridY = 18;

void ShaderFBOTestApp::setup()
{
	gl::Texture2d::Format fmt;
	fmt.setWrap(GL_REPEAT, GL_REPEAT);
	mNoiseBase = gl::Texture2d::create(loadImage(loadAsset("textures/TX_Noise_Base.png")), fmt);		//0
	mNoiseMid = gl::Texture2d::create(loadImage(loadAsset("textures/TX_Noise_Mid.png")), fmt);		//1
	mNoiseCoarse = gl::Texture2d::create(loadImage(loadAsset("textures/TX_Noise_Coarse.png")), fmt);	//2
	mNoiseFine = gl::Texture2d::create(loadImage(loadAsset("textures/TX_Noise_Fine.png")), fmt);		//3
	mNoiseGrad = gl::Texture2d::create(loadImage(loadAsset("textures/TX_Noise_Grad.png")), fmt);		//4
	mShader = gl::GlslProg::create(loadAsset("shaders/plasma.vert"), loadAsset("shaders/plasma.frag"));
	mShader->uniform("uTxBase", 0);
	mNoiseBase->bind(0);
	
	mShader->uniform("uTxMid", 1);
	mNoiseMid->bind(1);
	
	mShader->uniform("uTxCoarse", 2);
	mNoiseCoarse->bind(2);
	
	mShader->uniform("uTxFine", 3);
	mNoiseFine->bind(3);
	
	mShader->uniform("uTxGrad", 4);
	mNoiseGrad->bind(4);

	mFbo = gl::Fbo::create(kWidth, kHeight, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE)));
	mColorSrc = Surface8u::create(kWidth, kHeight, false);
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
	mShader->bind();
	mFbo->bindFramebuffer();
	gl::setMatricesWindow(getWindowSize());
	gl::clear(Color(0, 0, 0));
	gl::drawSolidRect(getWindowBounds());
	mFbo->unbindFramebuffer();
	
	auto colordef = gl::ShaderDef().color();
	auto colorShader = gl::getStockShader(colordef);
	colorShader->bind();
	drawPixelGrid(kGridX, kGridY, mFbo);
}

void ShaderFBOTestApp::drawPixelGrid(const int& gridX, const int& gridY, const gl::FboRef& mFbo)
{
	auto surf = mFbo->readPixels8u(Area(0, 0, mFbo->getWidth(), mFbo->getHeight()));

	int cellWidth = ceil(mColorSrc->getWidth() / static_cast<float>(gridX));
	int cellHeight = ceil(mColorSrc->getHeight() / static_cast<float>(gridY));

	for (int y = 0; y < gridY; y++)
	{
		float y0 = lmap<float>(y + 0.5f, 0, gridY, 0, kHeight);
		for (int x = 0; x < gridX; x++)
		{
			float x0 = -1;
			if (y % 2 == 0)
			{
				x0 = lmap<float>(x + 0.5f, 0, gridX, 0, kWidth);
			}
			else
			{
				x0 = lmap<float>(gridX - (x + 0.5f), 0, gridX, 0, kWidth);
			}
			auto pixel = surf.getPixel(ivec2(x0,y0));
			gl::color(pixel);
			gl::drawSolidRect(Rectf(x0 - cellWidth * 0.5f, y0 - cellHeight * 0.5f, x0 + cellWidth * 0.5f, y0 + cellHeight * 0.5f));
		}
	}
}

static void prepareSettings( App::Settings *settings )
{
	settings->setWindowSize( kWidth, kHeight );
}

CINDER_APP( ShaderFBOTestApp, RendererGl, *prepareSettings )
