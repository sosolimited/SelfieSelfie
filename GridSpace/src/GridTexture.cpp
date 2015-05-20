//
//  GridTexture.cpp
//
//  Created by Soso Limited on 5/20/15.
//
//

#include "GridTexture.h"

#include "cinder/ip/Resize.h"

using namespace soso;
using namespace cinder;

GridTexture::GridTexture( int iWidth, int iHeight, int iSubdivisions )
{
	cellDimensions = ivec2( iWidth, iHeight ) / iSubdivisions;
	columns = iSubdivisions;
	cells = iSubdivisions * iSubdivisions;

	texture = gl::Texture::create( iWidth, iHeight, gl::Texture::Format() );
}

void GridTexture::update( const ci::Surface &iSurface )
{
	auto offset = getIndexOffset( index );
	texture->update( iSurface, 0, offset );
	index = (index + 1) % cells;
}

ivec2 GridTexture::getIndexOffset( int iIndex ) const
{
	auto column = iIndex % columns;
	auto row = iIndex / columns;

	return ivec2( column, row ) * cellDimensions;
}
