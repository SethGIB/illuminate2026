#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"

using namespace ci;
using namespace ci::app;

class NormalMappingBasicApp : public App {
  public:	
	void	setup() override;
	void	resize() override;
	void	update() override;
	void	draw() override;
	
	CameraPersp			mCam;

	ci::TriMeshRef		mMesh;
	gl::BatchRef		mBatch;
	gl::TextureRef		mCamTex, mGradTex, mDiffuseTex, mNormalTex, mARMTex;
	gl::GlslProgRef		mGlsl;
	mat4				mCubeRotation;
	
	vec3				mLightPosWorldSpace;
};

void NormalMappingBasicApp::setup()
{
	mCam.lookAt( vec3( 0, 0, 4.8 ), vec3( 0,0.1,0 ) );

	mCamTex = gl::Texture::create(loadImage(loadAsset("TX_cam_0.png")), gl::Texture::Format().mipmap().wrapS(GL_REPEAT).wrapT(GL_REPEAT).loadTopDown(true));
	mCamTex->bind();
	mGradTex = gl::Texture::create(loadImage(loadAsset("TX_Grad_0.png")), gl::Texture::Format().mipmap().wrapS(GL_REPEAT).wrapT(GL_REPEAT).loadTopDown(true));
	mGradTex->bind(1);
	mDiffuseTex = gl::Texture::create( loadImage( loadAsset( "TX_brick_d.jpg" ) ), gl::Texture::Format().mipmap().wrapS(GL_REPEAT).wrapT(GL_REPEAT) );
	mDiffuseTex->bind(2);
	mNormalTex = gl::Texture::create( loadImage( loadAsset( "TX_brick_n.jpg" ) ), gl::Texture::Format().wrapS(GL_REPEAT).wrapT(GL_REPEAT));
	mNormalTex->bind( 3 );
	mARMTex = gl::Texture::create(loadImage(loadAsset("TX_brick_arm.jpg")), gl::Texture::Format().wrapS(GL_REPEAT).wrapT(GL_REPEAT));
	mARMTex->bind(4);

	mGlsl = gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );
	gl::ScopedGlslProg glslScp(mGlsl);
	mGlsl->uniform("uCamMap", 0);
	mGlsl->uniform("uGradMap", 1);
	mGlsl->uniform("uDiffuseMap", 2);
	mGlsl->uniform("uNormalMap", 3);
	mGlsl->uniform("uARMMap", 4);

	mLightPosWorldSpace = vec3(0, 4, 2.88);
	mGlsl->uniform("uLightLocViewSpace", vec3(mCam.getViewMatrix() * vec4(mLightPosWorldSpace, 1)));

	mBatch = gl::Batch::create(geom::Plane().origin(vec3(0, 0, 0)).normal(vec3(0,0,1)) >> geom::Transform( scale(vec3(1.667*3.0,3.0,1.0))), mGlsl);

	gl::enableDepthWrite();
	gl::enableDepthRead();


}

void NormalMappingBasicApp::resize()
{
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	gl::setMatrices( mCam );
}

void NormalMappingBasicApp::update()
{
	auto lx = 4.0f * math<float>::cos(getElapsedSeconds() * 0.1f);
	auto lz = 4.0f * lmap<float>(math<float>::sin(getElapsedSeconds() * 0.01f),-1.0,1.0,0.5,1.0);
	mLightPosWorldSpace = vec3(lx, 4, lz);
	mGlsl->uniform("uLightLocViewSpace", vec3(mCam.getViewMatrix() * vec4(mLightPosWorldSpace, 1)));
}

void NormalMappingBasicApp::draw()
{
	gl::clear();

	gl::setMatrices( mCam );
	gl::ScopedModelMatrix modelScope;
	//gl::multModelMatrix( mCubeRotation );
	mGlsl->uniform( "uLightLocViewSpace", vec3( mCam.getViewMatrix() * vec4( mLightPosWorldSpace, 1 )) );
	mBatch->draw();
}

static void prepareSettings( App::Settings *settings )
{
	settings->setWindowSize( 1280, 720 );
}

CINDER_APP( NormalMappingBasicApp, RendererGl, *prepareSettings )