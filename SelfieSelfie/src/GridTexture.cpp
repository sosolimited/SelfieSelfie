//
//  GridTexture.cpp
//
//  Created by Soso Limited on 5/20/15.
//
//

#include "GridTexture.h"

#include "cinder/ip/Resize.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include "cinder/app/App.h"

using namespace soso;
using namespace cinder;

GridTexture::GridTexture( const ci::ivec2 &iCellSize, int iRows )
{
  cellDimensions = iCellSize;
  blurredCellDimensions = cellDimensions / 6;

	columns = iRows;
	cells = columns * columns;


  auto total_blur_size = blurredCellDimensions * columns;
  auto total_focused_size = cellDimensions * columns;
  CI_LOG_I("Creating FBO for blurring: " << total_blur_size << ", focused: " << total_focused_size );

  auto color_format = gl::Texture::Format();
  auto fbo_format = gl::Fbo::Format().disableDepth().colorTexture( color_format );

	blurredBuffer = gl::Fbo::create( total_blur_size.x, total_blur_size.y, fbo_format );
  focusedBuffer = gl::Fbo::create( total_focused_size.x, total_focused_size.y, fbo_format );
  intermediateBlurBuffer = gl::Fbo::create( blurredCellDimensions.x, blurredCellDimensions.y, fbo_format );

	try {
    CI_LOG_I("Loading Downsampling Shaders");
		blurProg = gl::GlslProg::create( app::loadAsset( "blur.vs" ), app::loadAsset( "blur.fs" ) );
    passthroughProg = gl::GlslProg::create( app::loadAsset( "passthrough.vs" ), app::loadAsset( "passthrough.fs" ) );
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error compiling downsample shader: " << exc.what() );
	}
}

void GridTexture::update( const ci::Surface &iSurface )
{
	index = (index + 1) % cells;

  if( ! intermediateTexture ) {
    CI_LOG_I("Creating intermediate texture");
    intermediateTexture = gl::Texture::create( iSurface, gl::Texture::Format().loadTopDown().wrapS( GL_CLAMP_TO_EDGE ).wrapT( GL_CLAMP_TO_EDGE ) );
  }
  else {
    intermediateTexture->update( iSurface );
  }

  gl::ScopedTextureBind tex0( intermediateTexture, 0 );
  renderClearTexture();
  renderBlurredTexture();
}

ivec2 GridTexture::getIndexOffset( const ci::ivec2 &iCellDimensions, int iIndex ) const
{
	auto column = iIndex % columns;
	auto row = iIndex / columns;

	return ivec2( column, row ) * iCellDimensions;
}

void GridTexture::renderClearTexture()
{
  gl::ScopedFramebuffer buffer( focusedBuffer );
  gl::ScopedViewport view( getIndexOffset( cellDimensions, index ), cellDimensions );
  gl::ScopedGlslProg prog( passthroughProg );

  passthroughProg->uniform( "uSampler", 0 );

  gl::drawSolidRect( Rectf( -1, -1, 1, 1 ), vec2( 0, 0 ), vec2( 1, 1 ) );
}

void GridTexture::renderBlurredTexture()
{
  gl::ScopedGlslProg prog( blurProg );
  blurProg->uniform( "uSampler", 0 );
  { // First blur pass
    gl::ScopedFramebuffer buffer( intermediateBlurBuffer );
    gl::ScopedViewport    view( vec2(0), blurredCellDimensions );

    blurProg->uniform( "uTexelSize", vec2(1, 0) / vec2(blurredCellDimensions) );
    gl::drawSolidRect( Rectf( -1, -1, 1, 1 ), vec2( 0, 0 ), vec2( 1, 1 ) );
  }

  // Second blur pass.
  gl::ScopedFramebuffer buffer( blurredBuffer );
	gl::ScopedViewport view( getIndexOffset( blurredCellDimensions, index ), blurredCellDimensions );
  gl::ScopedTextureBind tex0( intermediateBlurBuffer->getColorTexture() );

  blurProg->uniform( "uTexelSize", vec2(0, 1) / vec2(blurredCellDimensions) );

	gl::drawSolidRect( Rectf( -1, -1, 1, 1 ), vec2( 0, 0 ), vec2( 1, 1 ) );
}
