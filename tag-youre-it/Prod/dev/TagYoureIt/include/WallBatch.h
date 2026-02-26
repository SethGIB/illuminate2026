#pragma once
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;
namespace TYI
{
	class WallBatch;
	typedef std::shared_ptr<WallBatch> WallBatchRef;

	class WallBatch
	{
	public:
		WallBatch();

		// load shaders from path and set uniforms
		void loadShaders(const std::string& vertPath, const std::string& fragPath, const vec2& texScale);

		// load textures from paths and bind to shader
		void loadTextures(const std::string& diffuse, const std::string& normal, const std::string& arm, const std::string &grad);
		void setupBatch();
		void setActive(const bool& active); //bind or unbind textures
		void step(const vec3& lightPos);
		void show();

		static WallBatchRef create()
		{
			return std::make_shared<WallBatch>();
		};

	private:
		gl::GlslProgRef mShader;
		gl::BatchRef mBatch;

		gl::Texture2dRef mDiffuseTex;
		gl::Texture2dRef mNormalTex;
		gl::Texture2dRef mArmTex;
		gl::Texture2dRef mGradientTex;
	};
}