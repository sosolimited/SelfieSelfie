#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "ParticleSystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace soso;

class FrameTimeApp : public App {
public:
	void setup() override;
	void update() override;

	void draw() override;
private:
	CaptureRef			_capture;
	gl::TextureRef	_frame_texture;
	ParticleSystem	_particles;
};

void FrameTimeApp::setup()
{
	console() << getWindowSize() << endl;

	_particles.setup();
	_particles.registerTouchEvents( getWindow() );

	_frame_texture = gl::Texture::create( loadImage( loadAsset( "water.png" ) ) );

	/*
	try {
		_capture = Capture::create( 320, 240 );
		_capture->start();
	} catch ( ci::Exception &exc ) {
		CI_LOG_E( "Error setting up capture: " << exc.what() );
	}
	*/
}

void FrameTimeApp::update()
{
	_particles.step();
}

void FrameTimeApp::draw()
{
	gl::clear( Color( 0, 0.5f, 0.5f ) );

	gl::ScopedMatrices matrices;
	gl::setMatricesWindowPersp( app::getWindowSize() );

	gl::ScopedTextureBind tex( _frame_texture, 0 );
	_particles.draw();
}

CINDER_APP( FrameTimeApp, RendererGl )
