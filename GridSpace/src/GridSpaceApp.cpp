#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "GridMesh.h"
#include "CameraLandscape.h"
#include "cinder/MotionManager.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

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
private:
	GridMesh				mesh;
	CameraLandscape landscape;
	CaptureRef			capture;
	CameraPersp			camera;
};

void GridSpaceApp::setup()
{
	mesh.setup();
	landscape.setup();
	MotionManager::enable();

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

		capture = Capture::create( 1920 / 2, 1080 / 2, front_facing_camera );
		capture->start();
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error using device camera: " << exc.what() );
	}
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
		landscape.updateTexture( *capture->getSurface() );
	}
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );
	landscape.draw();
	mesh.draw();

	/*
	gl::setMatricesWindowPersp( getWindowSize() );
	gl::translate( vec3( getWindowCenter(), 0 ) );
	gl::rotate( MotionManager::getRotation() );
	gl::drawCube( vec3( 0 ), vec3( 50, 100, 5 ) );

	gl::scale( vec3( 50 ) );
	gl::drawCoordinateFrame();
	*/
}

CINDER_APP( GridSpaceApp, RendererGl )
