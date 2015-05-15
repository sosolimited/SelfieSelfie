#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "GridMesh.h"
#include "cinder/MotionManager.h"

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
	CameraPersp			camera;
	const ci::vec3	target = vec3( 50, 5, 50 );
};

void GridSpaceApp::setup()
{
	mesh.setup();
	MotionManager::enable();

	camera.lookAt( vec3( 0 ), target, vec3( 0, 1, 0 ) );
	camera.setPerspective( 60, getWindowAspectRatio(), 0.1f, 1000 );
}

void GridSpaceApp::mouseDown( MouseEvent event )
{
}

void GridSpaceApp::update()
{
	if( MotionManager::isDataAvailable() ) {
		auto rotated_target = MotionManager::getRotationMatrix() * vec4( target, 0 );
		camera.lookAt( vec3( 0 ), vec3( rotated_target ), vec3( 0, 1, 0 ) );
	}
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );
	mesh.draw();

	gl::setMatricesWindowPersp( getWindowSize() );
	gl::translate( vec3( getWindowCenter(), 0 ) );
	gl::rotate( MotionManager::getRotation() );

	gl::drawVector( vec3( 0 ), target );
	gl::scale( vec3( 50 ) );
	gl::drawCoordinateFrame();
}

CINDER_APP( GridSpaceApp, RendererGl )
