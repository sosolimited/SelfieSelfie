//
//  Image.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "Image.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"

using namespace soso;
using namespace cinder;

Image::Image( const gl::TextureRef &iTexture )
: texture( iTexture ),
	size( app::toPoints( texture->getSize() ) )
{
	CI_LOG_I( "Image size: " << size );
}

void Image::draw()
{
	gl::ScopedAlphaBlend blend( true );

	auto placement = Rectf( position(), position() + size );

	if( alpha > 0.0f ) {

		if( backing.a > 0.0f ) {
			gl::ScopedColor color( backing * alpha );
			gl::drawSolidRect( placement );
		}

		gl::color( ColorA( tint ) * alpha );
		gl::draw( texture, placement );
	}
}
