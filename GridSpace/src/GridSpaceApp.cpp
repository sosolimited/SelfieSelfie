#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "GridMesh.h"
#include "CameraLandscape.h"
#include "cinder/MotionManager.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "GridTexture.h"
#include "TimeGrid.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

class GridSpaceApp : public App {
public:
	void setup() override;
	void touchesBegan( TouchEvent event ) override;
	void update() override;
	void draw() override;

private:
	GridMesh				mesh;
	CameraLandscape landscape;
	CaptureRef			capture;
	CameraPersp			camera;
	GridTextureRef	gridTexture;
	TimeGrid				timeGrid;

	bool						doDrawDebug = false;
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
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error using device camera: " << exc.what() );
	}

	landscape.setup( gridTexture->getBlurredTexture() );
	timeGrid.setup( gridTexture->getTexture() );
}

void GridSpaceApp::touchesBegan( TouchEvent event )
{
	doDrawDebug = ! doDrawDebug;
}

void GridSpaceApp::update()
{
	if( MotionManager::isDataAvailable() ) {
		auto r = MotionManager::getRotation();
		camera.setOrientation( r );
	}

	if( capture->checkNewFrame() ) {
		gridTexture->update( *capture->getSurface() );
	}
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );
	// TODO: bind both blurred and normal texture
	gl::ScopedTextureBind tex0( gridTexture->getBlurredTexture(), 0 );

	timeGrid.draw( gridTexture->getCurrentIndex() );
	landscape.draw( gridTexture->getCurrentIndex() );
	mesh.draw( gridTexture->getCurrentIndex() );

	if( doDrawDebug )
	{
		if( gridTexture->getTexture() )
		{
			auto size = vec2(192, 108) * 0.8f;
			gl::ScopedMatrices mat;
			gl::setMatricesWindow( app::getWindowSize() );
			gl::draw( gridTexture->getTexture(), Rectf( vec2(0), size ) );

			gl::translate( size * vec2(1, 0) );
			gl::draw( gridTexture->getBlurredTexture(), Rectf( vec2(0), size ) );
		}
	}

}

CINDER_APP( GridSpaceApp, RendererGl )
