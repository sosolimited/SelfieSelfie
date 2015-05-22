#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "GridMesh.h"
#include "CameraLandscape.h"
#include "cinder/MotionManager.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "GridTexture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

class GridSpaceApp : public App {
public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	void blurInput();
private:
	GridMesh				mesh;
	CameraLandscape landscape;
	CaptureRef			capture;
	CameraPersp			camera;
	GridTextureRef	gridTexture;
	gl::FboRef			blurredBuffer;
	gl::GlslProgRef	downsampleProg;
};

void GridSpaceApp::setup()
{
	mesh.setup();

	MotionManager::enable();

	GLint size;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
	CI_LOG_I( "Max texture size: " << size );

	auto target = vec3( 50, 5, 50 );
	camera.lookAt( vec3( 0 ), target, vec3( 0, 1, 0 ) );
	camera.setPerspective( 60, getWindowAspectRatio(), 0.1f, 1000 );

	try {
		auto front_facing_camera = ([] {
			auto &devices = Capture::getDevices();
			auto first_device = devices.front();
			for( auto device : devices ) {
				if( device->isFrontFacing() ) {
					return device;
				}
			}
			return first_device;
		}());

		capture = Capture::create( 480, 360, front_facing_camera );
		capture->start();
		const auto divisions = 8;
		const auto size = divisions * capture->getSize();
		gridTexture = make_shared<GridTexture>( size.x, size.y, divisions );

		auto color_format = gl::Texture::Format();
		auto fbo_format = gl::Fbo::Format().disableDepth().colorTexture( color_format );
		blurredBuffer = gl::Fbo::create( size.x / (divisions), size.y / (divisions), fbo_format );
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error using device camera: " << exc.what() );
	}

	downsampleProg = gl::GlslProg::create( loadAsset( "blur.vs" ), loadAsset( "blur.fs" ) );
//	landscape.setup( gridTexture->getTexture() );
	landscape.setup( blurredBuffer->getColorTexture() );
}

void GridSpaceApp::mouseDown( MouseEvent event )
{
}

void GridSpaceApp::update()
{
	if( MotionManager::isDataAvailable() ) {
		auto r = MotionManager::getRotation();
		camera.setOrientation( r );
	}

	if( capture->checkNewFrame() ) {
		gridTexture->update( *capture->getSurface() );
		blurInput();
	}
}

void GridSpaceApp::blurInput()
{
	auto index = gridTexture->getCurrentIndex();
	auto size = blurredBuffer->getSize() / 8;

	gl::ScopedMatrices matrices;
	gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );
	gl::ScopedGlslProg prog( downsampleProg );
	gl::ScopedViewport view( gridTexture->getIndexOffset( size, index ), size );
	gl::ScopedFramebuffer fbo( blurredBuffer );

	downsampleProg->uniform( "uSampler", 0 );
	downsampleProg->uniform( "uFrameIndex", (float)index );

	gl::drawSolidRect( Rectf( -1, -1, 1, 1 ), vec2( 0, 0 ), vec2( 1, 1 ) );
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );
	landscape.draw( gridTexture->getCurrentIndex() );
	mesh.draw();

	if( gridTexture->getTexture() ) {
		auto size = vec2(192, 108) * 0.8f;
		gl::ScopedMatrices mat;
		gl::setMatricesWindow( app::getWindowSize() );
		gl::draw( gridTexture->getTexture(), Rectf( vec2(0), size ) );

		gl::translate( size * vec2(1, 0) );
		gl::draw( blurredBuffer->getColorTexture(), Rectf( vec2(0), size ) );
	}
}

CINDER_APP( GridSpaceApp, RendererGl )
