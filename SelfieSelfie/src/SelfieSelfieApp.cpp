#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Log.h"
#include "cinder/gl/Fbo.h"

#include "GridTexture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

class SelfieSelfieApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;
private:

  CaptureRef			capture;

  GridTextureRef  gridTexture;
};

void SelfieSelfieApp::setup()
{
  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Pre-Setup gl error: " << gl::getErrorString(err) );
  }

  try {
    CI_LOG_I("Creating Grid Texture");
    gridTexture = make_shared<GridTexture>( ivec2(320, 240), 12 );
  }
  catch( ci::Exception &exc ) {
    CI_LOG_E( "Error creating grid texture: " << exc.what() );
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

    capture = Capture::create( 320, 240, front_facing_camera );
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
    gridTexture->update( *capture->getSurface() );

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

  if( gridTexture ) {
    auto rect = Rectf(gridTexture->getTexture()->getBounds());
    auto window_rect = Rectf(getWindowBounds());
    auto window_rect_top = window_rect.scaled( vec2( 1.0f, 0.5f ) );
    auto window_rect_bottom = window_rect_top + vec2( 0, window_rect_top.getHeight() );
    gl::draw( gridTexture->getTexture(), rect.getCenteredFit( window_rect_top, false ) );
    gl::draw( gridTexture->getBlurredTexture(), rect.getCenteredFit( window_rect_bottom, false ) );
  }

  gl::ScopedColor color( Color( 0.0f, 1.0f, 1.0f ) );
  gl::drawSolidCircle( getWindowCenter(), 20.0f );

  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Draw gl error: " << gl::getErrorString(err) );
  }
}

CINDER_APP( SelfieSelfieApp, RendererGl )
