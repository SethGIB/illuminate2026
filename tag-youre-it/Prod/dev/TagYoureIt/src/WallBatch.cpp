#include "WallBatch.h"

using namespace ci;
using namespace ci::app;

TYI::WallBatch::WallBatch()
{
}

void TYI::WallBatch::loadTextures(const std::string& diffuse, const std::string& normal, const std::string& arm, const std::string& grad)
{
	gl::Texture2d::Format texFormat = gl::Texture::Format().mipmap().wrapS(GL_REPEAT).wrapT(GL_REPEAT);
	mDiffuseTex = gl::Texture2d::create(loadImage(loadAsset(diffuse)), texFormat);
	mNormalTex = gl::Texture2d::create(loadImage(loadAsset(normal)), texFormat);
	mArmTex = gl::Texture2d::create(loadImage(loadAsset(arm)), texFormat);
	mGradientTex = gl::Texture2d::create(loadImage(loadAsset(grad)));

	mDiffuseTex->bind(0);
	mNormalTex->bind(1);
	mArmTex->bind(2);
	mGradientTex->bind(3);
}

void TYI::WallBatch::loadShaders(const std::string& vertPath, const std::string& fragPath, const vec2& texScale)
{
	mShader = gl::GlslProg::create(loadAsset(vertPath), loadAsset(fragPath));
	
	mShader->uniform("uWallDiffuseMap", 0);
	mShader->uniform("uWallNormalMap", 1);
	mShader->uniform("uWallArmMap", 2);
	mShader->uniform("uGradMap", 3);
	mShader->uniform("uCameraRgb", 4);
	mShader->uniform("uCameraDepth", 5);
	mShader->uniform("uTexScale", texScale);

}

void TYI::WallBatch::setupBatch()
{
	mBatch = gl::Batch::create(geom::Plane().origin(vec3(0, 0, 0)).normal(vec3(0, 0, 1)) >> geom::Transform(scale(vec3(3.0, 1.667 * 3.0, 1.0))), mShader);
}

void TYI::WallBatch::setActive(const bool& active)
{
	if (active)
	{
		mDiffuseTex->bind(0);
		mNormalTex->bind(1);
		mArmTex->bind(2);
		mGradientTex->bind(3);
	}
	else
	{
		mDiffuseTex->unbind(0);
		mNormalTex->unbind(1);
		mArmTex->unbind(2);
		mGradientTex->unbind(3);
	}
}

void TYI::WallBatch::step(const vec3& lightPos)
{
	mShader->uniform("uLightLocViewSpace", lightPos);
}

void TYI::WallBatch::show()
{
	mBatch->draw();
}
