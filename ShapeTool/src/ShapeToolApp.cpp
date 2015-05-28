#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

///
/// ShapeTool loads a curve from SVG and allows you to decimate it for
/// use as a series of time-sampled shapes in the main graphics application.
///
class ShapeToolApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ShapeToolApp::setup()
{
}

void ShapeToolApp::mouseDown( MouseEvent event )
{
}

void ShapeToolApp::update()
{
}

void ShapeToolApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( ShapeToolApp, RendererGl )
