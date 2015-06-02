#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"

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
}

void SelfieSelfieApp::update()
{
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
