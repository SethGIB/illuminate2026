#include "Plasma.h"


void Plasma::init(const std::string& vert, const std::string& frag, const std::string& noiseTex, const std::string& gradTex, const ivec2& fboDims)
{
	//load shader
	try
	{
		mShader = gl::GlslProg::create(loadAsset(vert), loadAsset(frag));
	}
	catch (const std::exception& e)
	{
		console() << "Shader failed to load: " << e.what() << std::endl;
	}
	//load textures
	try
	{
		gl::Texture2d::Format fmt;
		fmt.setWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
		mPlasmaTex = gl::Texture2d::create(loadImage(loadAsset(noiseTex)), fmt);
		mGradientTex = gl::Texture2d::create(loadImage(loadAsset(gradTex)), fmt);
	}
	catch (const std::exception& e)
	{
		console() << "Texture failed to load: " << e.what() << std::endl;
	}
	mRenderTarget = gl::Fbo::create(fboDims.x, fboDims.y, gl::Fbo::Format().colorTexture(gl::Texture2d::Format().internalFormat(GL_RGB).dataType(GL_UNSIGNED_BYTE)));
	mShader->uniform("uTxNoise", 0);
	mShader->uniform("uTxGrad", 1);
}

void Plasma::setActive(bool active)
{
	if(active)
	{
		mShader->bind();
		mPlasmaTex->bind(0);
		mGradientTex->bind(1);
	}
	else
	{
		gl::getStockShader(gl::ShaderDef().color())->bind();
		mPlasmaTex->unbind(0);
		mGradientTex->unbind(1);
	}
}

void Plasma::render(const ivec2& dims, const Area& bounds)
{
	mPlasmaTex->bind(0);
	mGradientTex->bind(1);
	mShader->bind();
	mRenderTarget->bindFramebuffer();
	gl::setMatricesWindow(dims);
	gl::clear(Color(0, 0, 0));
	gl::drawSolidRect(bounds);
	mRenderTarget->unbindFramebuffer();
	mPlasmaTex->unbind(0);
	mGradientTex->unbind(1);
	gl::getStockShader(gl::ShaderDef().color())->bind();
}

Surface8u Plasma::getPixels() const
{
	return mRenderTarget->readPixels8u(Area(0, 0, mRenderTarget->getWidth(), mRenderTarget->getHeight()));
}
