//
//  IntroSequence.cpp
//
//  Created by Soso Limited on 6/9/15.
//
//

#include "IntroSequence.h"
#include "cinder/ip/Premultiply.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Log.h"

using namespace soso;
using namespace cinder;
using namespace choreograph;

void IntroSequence::setup( const ci::fs::path &iImageBasePath )
{
	timeline.clear();
	items.clear();
	endTime = 0.0f;

	overlayColor = ColorA::hex( 0xffF8ED31 );
	backgroundColor = Color::gray( 0.12f );
	backgroundAlpha = 1.0f;

	if( ! iImageBasePath.empty() ) {
		CI_LOG_I( "Loading intro images from: " << iImageBasePath );
		showItem( iImageBasePath / "soso-logo.png", 1.0f );
		showItem( iImageBasePath / "selfie-logo.png", 2.0f );
		showBlank( 0.33f );
		auto cd = 0.9f;
		showItem( iImageBasePath / "countdown-3.png", cd );
		showItem( iImageBasePath / "countdown-2.png", cd );
		showItem( iImageBasePath / "countdown-1.png", cd );
	}
	else {
		CI_LOG_W( "Provided image directory is empty." );
	}

	showFlash();

	timeline.cue( [this] { handleFinish(); }, endTime );
	// Jump to where the logo is visible to avoid long blank screen.
	timeline.step( 0.2f );
}

void IntroSequence::showItem( const ci::fs::path &iPath, float duration )
{
	auto start = endTime;
	endTime += duration + 0.2f;

	auto surf = Surface( loadImage( app::loadAsset( iPath ) ) );
	if( ! surf.isPremultiplied() ) {
		ip::premultiply( &surf );
	}
	auto item = Image( gl::Texture::create( surf ) );
	auto size = ivec2( item.getSize() );
	auto position = vec2( app::getWindowSize() - size ) / 2.0f;
	item.setPosition( position );
	item.setTint( overlayColor );

	timeline.apply( item.getAlphaAnim() )
		.set( 0.0f )
		.hold( start )
		.then<RampTo>( 1.0f, 0.2f, ch::EaseOutQuad() )
		.hold( duration )
		.then<RampTo>( 0.0f, 0.2f );

	// move brings the ch::Output with it.
	items.emplace_back( std::move(item) );
}

void IntroSequence::showBlank( float duration )
{
	auto start = endTime;
	endTime += duration;
	timeline.append( &backgroundAlpha )
		.holdUntil( start )
		.then<RampTo>( 0.0f, 0.2f );
}

void IntroSequence::showFlash()
{
	auto start = endTime;
	endTime += 0.25f;

	timeline.append( &backgroundColor )
		.holdUntil( start )
		.then<RampTo>( Color::gray( 1.0f ), 0.1f, ch::EaseInBack() );

	timeline.append( &backgroundAlpha )
		.holdUntil( start + 0.075f )
		.set( 1.0f )
		.then<RampTo>( 0.0f, 1.5f, ch::EaseInOutSine() );
}

void IntroSequence::handleFinish()
{
	if( finishFn ) {
		finishFn();
	}

	items.clear();
}

void IntroSequence::start()
{
	CI_LOG_I("Starting Intro Sequence");
	updateConnection = app::App::get()->getSignalUpdate().connect([this] { update(); });
	timer.start();
}

void IntroSequence::stop()
{
	updateConnection.disconnect();
}

void IntroSequence::update()
{
	timeline.step( timer.getSeconds() );
	timer.start();
}

void IntroSequence::draw()
{
	gl::ScopedBlendPremult blend;

	if( backgroundAlpha > 0.0f ) {
		gl::ScopedColor color( ColorA( backgroundColor() ) * backgroundAlpha );
		gl::drawSolidRect( app::getWindowBounds() );
	}

	for( auto &item : items ) {
		item.draw();
	}
}
