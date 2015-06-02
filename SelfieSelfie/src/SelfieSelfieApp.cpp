#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SelfieSelfieApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;
private:

  CaptureRef			capture;
  gl::TextureRef	texture;
};

void SelfieSelfieApp::setup()
{
  try {
    CI_LOG_I("Setting up device camera.");
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

    capture = Capture::create( 640, 480, front_facing_camera );
    capture->start();
  }
  catch( ci::Exception &exc ) {
    CI_LOG_E( "Error using device camera: " << exc.what() );
  }
}

void SelfieSelfieApp::update()
{
  if( capture && capture->checkNewFrame() ) {
    #if defined(CINDER_ANDROID)
      texture = capture->getTexture();
    #else
      if( ! texture ) {
        texture = gl::Texture::create( *capture->getSurface(), gl::Texture::Format().loadTopDown() );
      }
      else {
        texture->update( *(capture->getSurface()) );
      }
    #endif
  }
}

void SelfieSelfieApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
  gl::drawSolidCircle( getWindowCenter(), 20.0f );

  if( texture ){
    gl::draw( texture );
  }
}

CINDER_APP( SelfieSelfieApp, RendererGl )
