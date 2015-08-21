//
//  Flash.cpp
//
//  Created by Soso Limited on 8/21/15.
//
//

#include "Flash.h"

using namespace soso;
using namespace cinder;

Flash::Flash( const vec2 &size )
: size( size )
{}

void Flash::flash( float duration )
{
	app::timeline().apply( &alpha, 1.0f, 0.0f, duration ).easeFn( EaseInOutSine() );
}

void Flash::draw()
{
	if( alpha() > 0.0f ) {
		gl::ScopedBlendPremult blend;
		gl::ScopedColor color(ColorA(alpha(), alpha(), alpha(), alpha()));
		gl::drawSolidRect( Rectf( vec2(0), size ) );
	}
}
