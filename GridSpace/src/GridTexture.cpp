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

GridTexture::GridTexture( int iWidth, int iHeight, int iSubdivisions )
{
	cellDimensions = ivec2( iWidth, iHeight ) / iSubdivisions;
	columns = iSubdivisions;
	cells = iSubdivisions * iSubdivisions;

	texture = gl::Texture::create( iWidth, iHeight, gl::Texture::Format().wrapS( GL_CLAMP_TO_EDGE ).wrapT( GL_CLAMP_TO_EDGE ) );

	// TODO: ensure blurred buffer size is evenly divisible by the number of subdivisions.
	auto color_format = gl::Texture::Format();
	auto fbo_format = gl::Fbo::Format().disableDepth().colorTexture( color_format );
	blurredBuffer = gl::Fbo::create( cellDimensions.x, cellDimensions.y, fbo_format );

	try {
		downsampleProg = gl::GlslProg::create( app::loadAsset( "blur.vs" ), app::loadAsset( "blur.fs" ) );
	}
	catch( ci::Exception &exc ) {
		CI_LOG_E( "Error compiling downsample shader: " << exc.what() );
	}
}

void GridTexture::update( const ci::Surface &iSurface )
{
	index = (index + 1) % cells;
	auto offset = getIndexOffset( cellDimensions, index );
	texture->update( iSurface, 0, offset );

	downsampleAndBlur();
}

ivec2 GridTexture::getIndexOffset( const ci::ivec2 &iCellDimensions, int iIndex ) const
{
	auto column = iIndex % columns;
	auto row = iIndex / columns;

	return ivec2( column, row ) * iCellDimensions;
}

void GridTexture::downsampleAndBlur()
{
	auto size = cellDimensions / 8;

	gl::ScopedMatrices matrices;
	gl::ScopedTextureBind tex0( texture, 0 );
	gl::ScopedGlslProg prog( downsampleProg );
	gl::ScopedViewport view( getIndexOffset( size, index ), size );
	gl::ScopedFramebuffer fbo( blurredBuffer );

	downsampleProg->uniform( "uSampler", 0 );
	downsampleProg->uniform( "uFrameIndex", (float)index );

	gl::drawSolidRect( Rectf( -1, -1, 1, 1 ), vec2( 0, 0 ), vec2( 1, 1 ) );
}
