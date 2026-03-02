#pragma once
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;

class Plasma;
typedef std::shared_ptr<Plasma> PlasmaRef;

class Plasma
{
public:
	Plasma() {}
	void init(const std::string &vert, const std::string &frag, const std::string &noiseTex, const std::string &gradTex, const ivec2& fboDims);
	void setActive(bool active);
	void render(const ivec2& dims, const Area& bounds);
	Surface8u getPixels() const;
	gl::Texture2dRef getTexture() const { return mRenderTarget->getColorTexture(); }
	static PlasmaRef create() { return std::make_shared<Plasma>(); }

private:
	gl::Texture2dRef mPlasmaTex;
	gl::Texture2dRef mGradientTex;
	gl::GlslProgRef mShader;
	gl::FboRef mRenderTarget;
};