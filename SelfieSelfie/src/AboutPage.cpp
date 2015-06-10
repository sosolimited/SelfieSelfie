//
//  AboutPage.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "AboutPage.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"

using namespace soso;
using namespace cinder;

void AboutPage::setup( const fs::path &iDirectory )
{
	description = std::unique_ptr<Image>( new Image( Surface( loadImage( app::loadAsset( iDirectory / "about-content.png" ) ) ) ) );
	iconBacking = std::unique_ptr<Image>( new Image( Surface( loadImage( app::loadAsset( iDirectory / "about-tab.png" ) ) ) ) );
	icon = std::unique_ptr<Image>( new Image( Surface( loadImage( app::loadAsset( iDirectory / "about-icon.png" ) ) ) ) );

	auto yellow = ColorA::hex( 0xffF8ED31 );

	auto window_size = vec2(app::getWindowSize());
	auto centered = (vec2( 0.5f ) * window_size) - (vec2( 0.5f ) * description->getSize());
	description->setPosition( centered );
	description->setBackingColor( ColorA::gray( 0.12f ) * 0.9f );
	description->setTint( yellow );
	description->setAlpha( 0.0f );
	description->setFullBleedBackground( true );

	auto bl = (vec2( 0.0f, 1.0f ) * window_size) - (vec2( 0.0f, 1.0f ) * icon->getSize());
	icon->setPosition( bl );
	icon->setTint( Color::white() );

	iconBacking->setPosition( bl );
	iconBacking->setTint( yellow );

	openButton = TouchArea::create( icon->getPlacement().scaledCentered( 5.5f ), [this] { showAbout(); } );
	closeButton = TouchArea::create( description->getPlacement(), [this] { showIcon(); } );

	closeButton->setEnabled( false );
}

void AboutPage::update()
{
	timeline->step( timer.getSeconds() );
	timer.start();
}

void AboutPage::show()
{
	visible = true;
	showIcon();
}

void AboutPage::hide()
{
	visible = false;
	description->setAlpha( 0.0f );
}

void AboutPage::draw()
{
	if( visible )
	{
		gl::ScopedAlphaBlend blend( true );

		iconBacking->draw();
		icon->draw();
		description->draw();
	}
}

void AboutPage::showAbout()
{
	openButton->setEnabled( false );
	closeButton->setEnabled( true );

	iconBacking->setAlpha( 0.0f );
	icon->setAlpha( 0.0f );
	description->setAlpha( 1.0f );
}

void AboutPage::showIcon()
{
	openButton->setEnabled( true );
	closeButton->setEnabled( false );

	iconBacking->setAlpha( 1.0f );
	icon->setAlpha( 1.0f );
	description->setAlpha( 0.0f );
}
