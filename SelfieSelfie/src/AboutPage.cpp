//
//  AboutPage.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "AboutPage.h"
#include "cinder/System.h"
#include "cinder/Utilities.h"

using namespace soso;
using namespace cinder;

void AboutPage::setup( const fs::path &iDirectory )
{
	description = std::unique_ptr<Image>( new Image( gl::Texture::create( loadImage( app::loadAsset( iDirectory / "about-tray.png" ) ) ) ) );
	icon = std::unique_ptr<Image>( new Image( gl::Texture::create( loadImage( app::loadAsset( iDirectory / "about-icon.png" ) ) ) ) );

	auto yellow = ColorA::hex( 0xffF8ED31 );

	auto window_size = vec2(app::getWindowSize());
	auto centered = (vec2( 0.5f ) * window_size) - (vec2( 0.5f ) * description->getSize());
	description->setPosition( centered );
	description->setBackingColor( Color::gray( 0.12f ) );
	description->setTint( yellow );
	description->setAlpha( 0.0f );

	auto bl = (vec2( 0.0f, 1.0f ) * window_size) - (vec2( 0.0f, 1.5f ) * icon->getSize());
	icon->setPosition( bl );
	icon->setBackingColor( yellow );
	icon->setTint( Color::gray( 1.0f ) );

	openButton = TouchArea::create( icon->getPlacement().scaled( 1.05f ), [this] { showAbout(); } );
	closeButton = TouchArea::create( description->getPlacement().scaled( vec2( 1.0f, 0.5f ) ) + (description->getSize() * vec2(0.0f, 0.5f)), [this] { hideAbout(); } );
	linkButton = TouchArea::create( description->getPlacement().scaled( vec2( 1.0f, 0.25f ) ), [this] { openLink(); } );

	closeButton->setEnabled( false );
	linkButton->setEnabled( false );
}

void AboutPage::update()
{
	timeline->step( timer.getSeconds() );
	timer.start();
}

void AboutPage::draw()
{
	gl::ScopedAlphaBlend blend( true );

	icon->draw();
	description->draw();
}

void AboutPage::showAbout()
{
	openButton->setEnabled( false );
	linkButton->setEnabled( true );
	closeButton->setEnabled( true );

	icon->setAlpha( 0.0f );
	description->setAlpha( 1.0f );
}

void AboutPage::hideAbout()
{
	openButton->setEnabled( true );
	linkButton->setEnabled( false );
	closeButton->setEnabled( false );

	icon->setAlpha( 1.0f );
	description->setAlpha( 0.0f );
}

void AboutPage::openLink()
{
	ci::launchWebBrowser( ci::Url( "http://sosolimited.com/?referrer=selfieselfie" ) );
}
