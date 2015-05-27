#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/MotionManager.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "GridMesh.h"
#include "CameraLandscape.h"
#include "GridTexture.h"
#include "TimeGrid.h"
#include "Landscape.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

struct TouchInfo {
	uint32_t  id;
	vec2			previous;
	vec2			position;
};

class GridSpaceApp : public App {
public:
	void setup() override;
	void touchesBegan( TouchEvent event ) override;
	void touchesMoved( TouchEvent event ) override;
	void touchesEnded( TouchEvent event ) override;

	void pinchStart();
	void pinchUpdate();
	void pinchEnd();

	void update() override;
	void draw() override;

private:
	GridMesh				mesh;
	/*
	CameraLandscape cameraLandscape;
	TimeGrid				timeGrid;
	*/
	CaptureRef			capture;
	CameraPersp			camera;
	GridTextureRef	gridTexture;
	Landscape				landscape;

	bool						doDrawDebug = false;

	vector<TouchInfo> touches;
	ci::vec3					cameraOffset;
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

	landscape.setup();
	/*
	cameraLandscape.setup( gridTexture->getBlurredTexture() );
	timeGrid.setup( gridTexture->getTexture() );
	*/
}

void GridSpaceApp::touchesBegan( TouchEvent event )
{
	for( auto &t : event.getTouches() ) {
		touches.push_back( { t.getId(), t.getPos(), t.getPos() } );
	}

	if( touches.size() == 2 ) {
		pinchStart();
	}
}

void GridSpaceApp::touchesMoved( TouchEvent event )
{
	for( auto &t : event.getTouches() ) {
		for( auto &s : touches ) {
			if( s.id == t.getId() ) {
				s.previous = s.position;
				s.position = t.getPos();
			}
		}
	}

	if( touches.size() == 2 ) {
		pinchUpdate();
	}
}

void GridSpaceApp::touchesEnded( TouchEvent event )
{
	touches.erase( std::remove_if( touches.begin(), touches.end(), [&event] (const TouchInfo &s) {
		for( auto &t : event.getTouches() ) {
			if (t.getId() == s.id) {
				return true;
			}
		}
		return false;
	}), touches.end() );
}

void GridSpaceApp::pinchStart()
{
}

void GridSpaceApp::pinchUpdate()
{
	auto base = distance(touches.at( 0 ).previous, touches.at( 1 ).previous);
	auto current = distance(touches.at( 0 ).position, touches.at( 1 ).position);

	auto delta = current - base;
	if( isfinite( delta ) )
	{
		auto ray = camera.getViewDirection();
		cameraOffset += delta * ray;
	}
}

void GridSpaceApp::update()
{
	camera.setEyePoint( cameraOffset * 0.1f );

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
	// TODO: bind both blurred and normal texture and avoid rebinding textures elsewhere.
	gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );
	gl::ScopedTextureBind tex1( gridTexture->getBlurredTexture(), 1 );

	landscape.draw( gridTexture->getCurrentIndex() );
	/*
	timeGrid.draw( gridTexture->getCurrentIndex() );
	cameraLandscape.draw( gridTexture->getCurrentIndex() );
	*/
	mesh.draw( gridTexture->getCurrentIndex() );

	if( doDrawDebug )
	{
		if( gridTexture->getTexture() )
		{
			auto size = vec2(getWindowSize()) * vec2(1.0, 0.5);
			gl::ScopedMatrices mat;
			gl::setMatricesWindow( app::getWindowSize() );
			gl::draw( gridTexture->getTexture(), Rectf( vec2(0), size ) );

			gl::translate( size * vec2(0, 1) );
			gl::draw( gridTexture->getBlurredTexture(), Rectf( vec2(0), size ) );
		}
	}

}

CINDER_APP( GridSpaceApp, RendererGl )
