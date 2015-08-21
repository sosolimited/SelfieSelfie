//
//  AboutPage.cpp
//
//  Created by Soso Limited on 6/10/15.
//
//

#include "AboutPage.h"
#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/gl/Scoped.h"

using namespace soso;
using namespace cinder;

const auto yellow = ColorA::hex( 0xffF8ED31 );

#pragma mark - NestingButton

NestingButton::NestingButton( const DataSourceRef &iIcon, const DataSourceRef &iBacking, const std::function<void ()> &iCallback )
: icon( new Image( Surface( loadImage( iIcon ) ) ) ),
	iconBacking( new Image( Surface( loadImage( iBacking ) ) ) )
{
	auto window_size = vec2(app::getWindowSize());

	openPosition = (vec2( 1.0f, 0.0f ) * window_size) - (vec2( 1.0f, 0.0f ) * icon->getSize());
	closedPosition = (vec2( 1.0f, 0.0f ) * window_size) - (vec2( 0.2f, 0.0f ) * icon->getSize());
	position = closedPosition;

	icon->setPosition( position );
	icon->setTint( Color::black() );

	iconBacking->setPosition( position );
	iconBacking->setTint( yellow );

	openButton = TouchArea::create( Rectf( openPosition, openPosition + icon->getSize() ).scaledCentered( 1.5f ), iCallback );
}

void NestingButton::draw() const
{
	iconBacking->draw();
	icon->draw();
}

void NestingButton::show( ci::Timeline &iTimeline  )
{
	hidden = false;
	iTimeline.apply( &position, openPosition, 0.32f, EaseInOutQuad() )
		.updateFn( [this] {
			icon->setPosition( position );
			iconBacking->setPosition( position );
		} );
}

void NestingButton::hide( ci::Timeline &iTimeline  )
{
	hidden = true;
	iTimeline.apply( &position, closedPosition, 0.4f, EaseInOutQuad() )
		.updateFn( [this] {
			icon->setPosition( position );
			iconBacking->setPosition( position );
		} );
}

#pragma mark - AboutPage

void AboutPage::setup( const fs::path &iDirectory )
{
	description = std::unique_ptr<Image>( new Image( Surface( loadImage( app::loadAsset( iDirectory / "about-content.png" ) ) ) ) );
	nestingButton = std::unique_ptr<NestingButton>( new NestingButton( app::loadAsset( iDirectory / "about-icon.png" ), app::loadAsset( iDirectory / "about-tab.png" ), [this] { handleIconClick(); } ) );
	screenshotInstructions = std::make_unique<Image>( Surface( loadImage( app::loadAsset( iDirectory / "instructions-popup.png" ) ) ) );

	auto window_size = vec2(app::getWindowSize());
	auto transparent_gray = ColorA::gray( 0.12f ) * 0.9f;
	auto desc_pos = (vec2( 1.0f, 0.0f ) * window_size) - (vec2( 1.0f, 0.0f ) * description->getSize());
	description->setPosition( desc_pos );
	description->setBackingColor( transparent_gray );
	description->setTint( yellow );
	description->setAlpha( 0.0f );
	screenshotInstructions->setTint( yellow );
	screenshotInstructions->setBackingColor( transparent_gray );

	closeButton = TouchArea::create( description->getPlacement(), [this] { hideAbout(); } );

	closeButton->setEnabled( false );
	nestingButton->setEnabled( false );
}

void AboutPage::update()
{
	timeline->step( timer.getSeconds() );
	timer.start();

	if( timeline->empty() ) {
		timeline->stepTo( 0.0 );
	}
}

void AboutPage::show()
{
	visible = true;
	nestingButton->setEnabled( true );

	showIcon();

	auto offscreen = vec2(0, - screenshotInstructions->getSize().y);
	app::timeline().apply( screenshotInstructions->getPositionAnim(), offscreen, vec2(0), 0.5f, EaseOutQuad() )
		.startFn( [this] { screenshotInstructions->setAlpha( 1.0f ); } );
	app::timeline().appendTo( screenshotInstructions->getPositionAnim(), offscreen, 0.5f, EaseInOutQuad() )
		.delay( 3.0f )
		.finishFn( [this] { screenshotInstructions->setAlpha( 0.0f ); } );
}

void AboutPage::hide()
{
	visible = false;
	description->setAlpha( 0.0f );
	nestingButton->setEnabled( false );
	nestingButton->hide( *timeline );

	auto offscreen = vec2(0, - screenshotInstructions->getSize().y);
	app::timeline().appendTo( screenshotInstructions->getPositionAnim(), offscreen, 0.5f, EaseInOutQuad() )
		.delay( 3.0f )
		.finishFn( [this] { screenshotInstructions->setAlpha( 0.0f ); } );
}

void AboutPage::draw()
{
	if( visible )
	{
		gl::ScopedBlendPremult blend;

		nestingButton->draw();
		screenshotInstructions->draw();
		description->draw();
	}
}

void AboutPage::handleIconClick()
{
	if( nestingButton->isHidden() ) {
		showIcon();
	}
	else {
		showAbout();
	}
}

void AboutPage::showAbout()
{
	nestingButton->setEnabled( false );
	nestingButton->hide( *timeline );
	closeButton->setEnabled( true );

	description->setAlpha( 1.0f );
}

void AboutPage::hideAbout()
{
	description->setAlpha( 0.0f );
	closeButton->setEnabled( false );
	nestingButton->setEnabled( true );
}

void AboutPage::showIcon()
{
	nestingButton->show( *timeline );

	if( hideCue ) {
		hideCue->removeSelf();
	}

	hideCue = timeline->add( [this] { nestingButton->hide( *timeline ); }, 15.0f );
}
