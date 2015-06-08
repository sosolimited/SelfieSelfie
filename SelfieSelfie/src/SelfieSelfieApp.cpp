#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Capture.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Log.h"
#include "cinder/gl/Fbo.h"

#include "GridTexture.h"
#include "Landscape.h"

#include "cinder/MotionManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace soso;

struct TouchInfo {
	TouchInfo() = default;
	TouchInfo( uint32_t iId, const vec2 &iStart, const vec2 &iPosition )
	: id( iId ),
		previous( iStart ),
		position( iPosition )
	{}

  uint32_t		id = 0;
  vec2				previous;
  vec2				position;
};

class SelfieSelfieApp : public App {
public:
	void setup() override;
	void update() override;
	void draw() override;

  void touchesBegan( TouchEvent event ) override;
  void touchesMoved( TouchEvent event ) override;
  void touchesEnded( TouchEvent event ) override;

	void updateCamera();

private:
	CameraPersp				camera;
  CaptureRef				capture;

  GridTextureRef		gridTexture;
  Landscape					landscape;
	gl::BatchRef			cameraImage;

  vector<TouchInfo> touches;
  ci::vec3					cameraOffset;
	float							cameraVelocity = 0.0f;

  bool							drawDebug = false;

};

void SelfieSelfieApp::setup()
{
  CI_LOG_I("Setting up selfie_x_selfie");

  MotionManager::enable();

  GLint size;
  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &size );
  CI_LOG_I( "Max texture size: " << size );

  auto target = vec3( 5, 0, 0 );
  camera.lookAt( vec3( 0 ), target, vec3( 0, 1, 0 ) );
  camera.setPerspective( 80, getWindowAspectRatio(), 0.1f, 50.0f );

  try {
    CI_LOG_I( "Initializing hardware camera." );
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

    CI_LOG_I( "Creating Grid Texture" );
    gridTexture = make_shared<GridTexture>( ivec2( 320, 240 ), 12 );

    CI_LOG_I( "Setting up landscape geometry." );
    landscape.setup();
    landscape.setTextureUnits( 0, 1 );
    landscape.setGridSize( gridTexture->getGridDimensions() );
  }
  catch( ci::Exception &exc ) {
    CI_LOG_E( "Error using device camera: " << exc.what() );
  }

  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Post-Setup gl error: " << gl::getErrorString(err) );
  }
}

void SelfieSelfieApp::touchesBegan( TouchEvent event )
{
  for( auto &t : event.getTouches() ) {
    touches.emplace_back( t.getId(), t.getPos(), t.getPos() );
  }

  if( touches.size() == 4 ) {
    drawDebug = ! drawDebug;
  }
}

void SelfieSelfieApp::touchesMoved( TouchEvent event )
{
  for( auto &t : event.getTouches() ) {
    for( auto &s : touches ) {
      if( s.id == t.getId() ) {
				s.previous = s.position;
        s.position = t.getPos();
      }
    }
  }
}

void SelfieSelfieApp::touchesEnded( TouchEvent event )
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

void SelfieSelfieApp::update()
{
	updateCamera();

  if( capture && capture->checkNewFrame() ) {
    gridTexture->update( *capture->getSurface() );

    auto err = gl::getError();
    if( err ) {
      CI_LOG_E( "Texture copy gl error: " << gl::getErrorString(err) );
    }
  }
}

void SelfieSelfieApp::updateCamera()
{
	if( touches.size() == 1 )
	{
		const auto forward = ([] {
			auto gravity = MotionManager::getGravityDirection();
			if( abs( gravity.x ) > abs( gravity.y ) ) {
				auto x = copysign( 1.0f, gravity.x );
				return vec3( x, 0, 0 );
			}
			else {
				auto y = copysign( 1.0f, gravity.y );
				return vec3( 0, - y, 0 );
			}
		}());

		auto &touch = touches.front();
		auto delta = vec3( touch.position - touch.previous, 0 );
		touch.previous = touch.position;

		auto amount = length( delta );
		auto dir = dot( normalize(delta), forward );

		if( isfinite( amount ) && isfinite( dir ) ) {
			cameraVelocity += amount * dir;
		}
	}

	auto ray = camera.getViewDirection();
	cameraOffset += ray * cameraVelocity * 0.0025f;
	cameraVelocity *= 0.86f;

	auto l = length(cameraOffset);
	auto maximum = 2.7f;
	if( l > maximum ) {
		cameraOffset *= (maximum / l);
	}
	camera.setEyePoint( cameraOffset );

  if( MotionManager::isDataAvailable() ) {
    auto r = MotionManager::getRotation();
    camera.setOrientation( r );
  }
}

void SelfieSelfieApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

  {
    gl::enableDepthRead();
    gl::enableDepthWrite();
		gl::ScopedGlslProg prog( gl::getStockShader( gl::ShaderDef().texture(gridTexture->getTexture()->getTarget()) ) );

    gl::ScopedTextureBind tex0( gridTexture->getTexture(), 0 );
    gl::ScopedTextureBind tex1( gridTexture->getBlurredTexture(), 1 );
    gl::ScopedMatrices matrices;
    gl::setMatrices( camera );

		landscape.draw( gridTexture->getCurrentIndex() );

		auto mat = translate( vec3( 0, -4.0f, 0 ) );
		auto dims = vec2(gridTexture->getCellDimensions()) / gridTexture->getGridSize();
		auto offset = vec2(gridTexture->getIndexOffset( gridTexture->getCellDimensions(), gridTexture->getCurrentIndex() )) / gridTexture->getGridSize();
		auto rect = Rectf( -1.0f, -1.0f, 1.0f, 1.0f ).scaled( vec2( 1.333f, 1.0f ) ).scaled( 0.2f );
		auto half_pi = (float) M_PI / 2.0f;
		auto xf1 = translate( vec3( - 4.0f, 0.0f, 0.0f ) ) * rotate( - half_pi, vec3( 1, 0, 0 ) ) * rotate( half_pi, vec3( 0, 1, 0 ) );
		auto xf2 = translate( vec3( 4.0f, 0.0f, 0.0f ) ) * rotate( half_pi, vec3( 1, 0, 0 ) ) * rotate( - half_pi, vec3( 0, 1, 0 ) );

		if( false )
		{
			gl::ScopedModelMatrix m;
			gl::multModelMatrix( xf1 );
			gl::drawSolidRect( rect, offset, offset + dims );
		}

		{
			gl::ScopedModelMatrix m;
			gl::multModelMatrix( xf2 );
			gl::drawSolidRect( rect, offset, offset + dims );
		}

  }

  gl::disableDepthRead();

  if( drawDebug && gridTexture )
  {
    gl::setMatricesWindow( getWindowSize() );

    auto rect = Rectf(gridTexture->getTexture()->getBounds());
    auto window_rect = Rectf(getWindowBounds());
    auto window_rect_a = window_rect.scaled( vec2( 0.5f ) );
    auto window_rect_b = window_rect_a + vec2( window_rect_a.getWidth(), 0 );
    gl::draw( gridTexture->getTexture(), rect.getCenteredFit( window_rect_a, false ) );
    gl::draw( gridTexture->getBlurredTexture(), rect.getCenteredFit( window_rect_b, false ) );
  }

  /*
  // For confirming version changes, draw a different colored dot.
  gl::ScopedColor color( Color( 1.0f, 0.0f, 1.0f ) );
  gl::drawSolidCircle( vec2( 20.0f ), 10.0f );
  */

  auto err = gl::getError();
  if( err ) {
    CI_LOG_E( "Draw gl error: " << gl::getErrorString(err) );
  }
}

inline void prepareSettings( app::App::Settings *iSettings )
{
  iSettings->setMultiTouchEnabled();
}

CINDER_APP( SelfieSelfieApp, RendererGl, &prepareSettings )
