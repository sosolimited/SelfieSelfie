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
	GridMesh		mesh;
	CameraPersp	camera;
	ci::quat		rotationCorrection;
};

void GridSpaceApp::setup()
{
	mesh.setup();
	MotionManager::enable();

	camera.lookAt( vec3( 0 ), vec3( 50, 5, 50 ), vec3( 0, 1, 0 ) );
	camera.setPerspective( 35, getWindowAspectRatio(), 0.1f, 1000 );
}

void GridSpaceApp::mouseDown( MouseEvent event )
{
}

void GridSpaceApp::update()
{
}

void GridSpaceApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::setMatrices( camera );
	gl::rotate( MotionManager::getRotation() );

	mesh.draw();
}

CINDER_APP( GridSpaceApp, RendererGl )
