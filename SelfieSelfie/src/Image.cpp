//
//  Image.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "Image.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"
#include "cinder/ip/Premultiply.h"

using namespace soso;
using namespace cinder;

Image::Image( Surface &&surface )
: size( app::toPoints( surface.getSize() ) )
{
	if( ! surface.isPremultiplied() ) {
		ip::premultiply( &surface );
	}
	texture = gl::Texture::create( surface );
}

Image::Image( const gl::TextureRef &iTexture )
: texture( iTexture ),
	size( app::toPoints( texture->getSize() ) )
{}

void Image::draw()
{
	gl::ScopedAlphaBlend blend( true );

	const auto placement = getPlacement();

	if( alpha > 0.0f ) {

		if( backing.a > 0.0f ) {
			gl::ScopedColor color( backing * alpha );
			if( fullBleedBackground ) {
				gl::drawSolidRect( app::getWindowBounds() );
			}
			else {
				gl::drawSolidRect( placement );
			}
		}

		gl::color( ColorA( tint ) * alpha );
		gl::draw( texture, placement );
	}
}
