#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
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
  gl::GlslProgRef shader;
};

void SelfieSelfieApp::setup()
{
  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Pre-Setup gl error: " << gl::getErrorString(err) );
  }

  try {
    shader = gl::GlslProg::create( loadAsset("blur.vs"), loadAsset("blur.fs") );
    CI_LOG_I("Loaded blur shader.");
  }
  catch ( ci::Exception &exc ) {
    CI_LOG_E( "Error loading blur shader: " << exc.what() );
  }

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
    CI_LOG_I("Device Camera set up.");
  }
  catch( ci::Exception &exc ) {
    CI_LOG_E( "Error using device camera: " << exc.what() );
  }

  err = gl::getError();
  if( err ) {
    CI_LOG_E( "Post-Setup gl error: " << gl::getErrorString(err) );
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

    auto err = gl::getError();
    if( err ) {
      CI_LOG_E( "Texture copy gl error: " << gl::getErrorString(err) );
    }
  }
}

void SelfieSelfieApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatricesWindow( getWindowSize() );

  if( texture ){
    gl::ScopedGlslProg    prog( shader );
    gl::ScopedTextureBind tex0( texture, 0 );

    shader->uniform( "uSampler", 0 );

    gl::drawSolidRect( Rectf( -1, -1, 1, 1 ) );
  }

  if( texture ) {
    gl::draw( texture );
  }



  gl::drawSolidCircle( getWindowCenter(), 20.0f );

  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Draw gl error: " << gl::getErrorString(err) );
  }
}

CINDER_APP( SelfieSelfieApp, RendererGl )
