//
//  Flash.cpp
//
//  Created by Soso Limited on 8/21/15.
//
//

#include "Flash.h"
#include "cinder/gl/draw.h"
#include "cinder/gl/scoped.h"

using namespace soso;
using namespace cinder;
using namespace choreograph;

Flash::Flash( const vec2 &size )
: size( size )
{}

void Flash::flash( float duration )
{
	sharedTimeline().apply( &alpha )
		.set( 1.0f )
		.then<RampTo>( 0.0f, duration, EaseInOutSine() );
}

void Flash::draw()
{
	if( alpha() > 0.0f ) {
		gl::ScopedBlendPremult blend;
		gl::ScopedColor color(ColorA(alpha(), alpha(), alpha(), alpha()));
		gl::drawSolidRect( Rectf( vec2(0), size ) );
	}
}
