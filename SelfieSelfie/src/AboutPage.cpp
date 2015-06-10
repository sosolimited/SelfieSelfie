//
//  AboutPage.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "AboutPage.h"

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
}

void AboutPage::draw()
{
	gl::ScopedAlphaBlend blend( true );

	icon->draw();
	description->draw();
}
