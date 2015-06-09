//
//  IntroSequence.cpp
//
//  Created by Soso Limited on 6/9/15.
//
//

#include "IntroSequence.h"
#include "cinder/ip/Premultiply.h"
#include "cinder/Log.h"

using namespace soso;
using namespace cinder;

void IntroSequence::setup( const ci::fs::path &iImageBasePath )
{
	timeline->clear();
	timeline->reset();

	if( fs::is_directory( iImageBasePath ) ) {
		CI_LOG_I( "Loading intro images from: " << iImageBasePath );
		showItem( iImageBasePath / "soso-logo.png", 2.0f );
		showItem( iImageBasePath / "selfie-logo.png", 3.0f );
		auto cd = 0.9f;
		showItem( iImageBasePath / "countdown-3.png", cd );
		showItem( iImageBasePath / "countdown-2.png", cd );
		showItem( iImageBasePath / "countdown-1.png", cd );
		showFlash();
	}

	timeline->add( [this] { handleFinish(); }, timeline->getEndTime() + 2.0f );
}

void IntroSequence::showItem( const ci::fs::path &iPath, float duration )
{
	auto start = endTime;
	endTime += duration + 0.2f;

	auto surf = Surface(loadImage( iPath ));
	if( ! surf.isPremultiplied() ) {
		ip::premultiply( &surf );
	}
	auto item = SequenceItem( gl::Texture::create( surf ) );
	auto size = app::toPoints( item.texture->getSize() );
	auto position = vec2( app::getWindowSize() - size ) / 2.0f;
	item.placement = Rectf( position, position + vec2( size ) );

	timeline->apply( &item.alpha, 1.0f, 0.2f ).easeFn( EaseOutQuad() ).startTime( start );
	timeline->appendTo( &item.alpha, 0.0f, 0.2f ).delay( duration );

	items.push_back( item ); // copy brings the anim with it (move makes this clearer in Choreograph)
}

void IntroSequence::showFlash()
{
	auto start = endTime;
	endTime += 0.2f;

	timeline->apply( &backgroundColor, Color::gray( 1.0f ), 0.1f ).easeFn( EaseInBack() ).startTime( start );
	timeline->apply( &backgroundAlpha, 0.0f, 1.5f ).easeFn( EaseInOutSine() ).startTime( start + 0.05f );
}

void IntroSequence::handleFinish()
{
	if( finishFn ) {
		finishFn();
	}

	items.clear();
}

void IntroSequence::update()
{
	timeline->step( timer.getSeconds() );
	timer.start();
}

void IntroSequence::draw()
{
	if( backgroundAlpha == 0.0f ) {
		return;
	}

	gl::ScopedAlphaBlend blend( true );

	gl::ScopedColor color( ColorA( backgroundColor() ) * backgroundAlpha );
	gl::drawSolidRect( app::getWindowBounds() );

	for( auto &item : items ) {
		if( item.alpha > 0.0f ) {
			gl::color( overlayColor * item.alpha() );
			gl::draw( item.texture, item.placement );
		}
	}
}
