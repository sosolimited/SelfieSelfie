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

	try {
		_capture = Capture::create( 320, 240 );
		_capture->start();
	} catch ( ci::Exception &exc ) {
		CI_LOG_E( "Error setting up capture: " << exc.what() );
	}
}

void FrameTimeApp::update()
{
	_particles.step();

	if( _capture && _capture->checkNewFrame() ) {
		if( ! _frame_texture ) {
			// Capture images come back as top-down, and it's more efficient to keep them that way
			_frame_texture = gl::Texture::create( *_capture->getSurface(), gl::Texture::Format().loadTopDown() );
		}
		else {
			_frame_texture->update( *_capture->getSurface() );
		}
	}
}

void FrameTimeApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	gl::ScopedMatrices matrices;
	gl::setMatricesWindowPersp( app::getWindowSize() );

	if( _frame_texture ) {
		gl::ScopedTextureBind tex( _frame_texture, 0 );
		_particles.draw();
	}
}

CINDER_APP( FrameTimeApp, RendererGl )
