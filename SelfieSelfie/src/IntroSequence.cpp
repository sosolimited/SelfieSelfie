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

	timeline->apply( &item.alpha, 1.0f, 0.3f ).easeFn( EaseOutQuad() ).startTime( start );
	timeline->appendTo( &item.alpha, 0.0f, 0.2f ).delay( duration );

	items.push_back( item ); // copy brings the anim with it (move makes this clearer in Choreograph)
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
	gl::ScopedAlphaBlend blend( true );

	for( auto &item : items ) {
		if( item.alpha > 0.0f ) {
			gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) * item.alpha() );
			gl::draw( item.texture, item.placement );
		}
	}
}
