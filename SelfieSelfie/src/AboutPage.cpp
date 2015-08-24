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
#include "SharedTimeline.h"

using namespace soso;
using namespace cinder;
using namespace choreograph;

using ch::detail::make_unique;

const auto yellow = ColorA::hex( 0xffF8ED31 );

#pragma mark - NestingButton

NestingButton::NestingButton( std::unique_ptr<Image> &&iIcon, const std::function<void ()> &iCallback, const vec2 &iOpenPosition )
: icon( std::move(iIcon) ),
	openPosition( iOpenPosition )
{
	closedPosition = vec2(app::getWindowWidth() - icon->getSize().x * 0.1f, openPosition.y);

	icon->setPosition( closedPosition );
	icon->setBackingColor( yellow );
	icon->setTint( Color::black() );

	touchArea = TouchArea::create( Rectf( openPosition, openPosition + icon->getSize() ).scaledCentered( 2.0f ), iCallback );
}

void NestingButton::draw() const
{
	icon->draw();
}

void NestingButton::show( ch::Timeline &iTimeline  )
{
	hidden = false;

	iTimeline.apply( icon->getPositionAnim() )
		.then<RampTo>( openPosition, 0.32f, ch::EaseInOutQuad() );
}

void NestingButton::hide( ch::Timeline &iTimeline  )
{
	hidden = true;
	iTimeline.apply( icon->getPositionAnim() )
		.then<RampTo>( closedPosition, 0.4f, ch::EaseInQuad() );
}

#pragma mark - AboutPage

void AboutPage::setup( const fs::path &iDirectory )
{
	auto window_size = vec2(app::getWindowSize());
	auto button_image = make_unique<Image>( Surface( loadImage( app::loadAsset(
	iDirectory /
	"about-button.png" ) ) ) );
	auto baseline = app::getWindowHeight() * 0.1f;
	auto padding = std::max( app::getWindowWidth() * 0.05f, 10.0f );
	auto right = app::getWindowWidth() - padding;

	description = std::unique_ptr<Image>( new Image( Surface( loadImage( app::loadAsset( iDirectory / "about-content.png" ) ) ) ) );

	auto button_pos = vec2(right - button_image->getSize().x, baseline - button_image->getSize().y);
	nestingButton = ch::detail::make_unique<NestingButton>( std::move(button_image), [this] { handleIconClick(); }, button_pos );
	screenshotInstructions = make_unique<Image>( Surface( loadImage( app::loadAsset( iDirectory / "instructions-popup.png" ) ) ) );
	instructionsPosition = vec2((window_size.x - screenshotInstructions->getSize().x) / 2.0, baseline - screenshotInstructions->getSize().y);
	screenshotInstructions->setPosition( instructionsPosition );

	auto transparent_gray = ColorA::gray( 0.12f ) * 0.9f;
	auto desc_pos = (window_size - description->getSize()) * 0.5f;
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
	timeline.step( timer.getSeconds() );
	timer.start();
}

void AboutPage::show()
{
	visible = true;
	nestingButton->setEnabled( true );

	auto info = showIcon();
	auto instructions_start = info.getItem().getEndTime() + 0.4f;
	screenshotInstructions->setAlpha( 0.0f );

	auto offscreen = vec2(instructionsPosition.x, - screenshotInstructions->getSize().y);
	sharedTimeline().apply( screenshotInstructions->getPositionAnim() )
		.startFn( [this] (Motion<vec2> &m) { screenshotInstructions->setAlpha( 1.0f ); } )
		.finishFn( [this] (Motion<vec2> &m) { screenshotInstructions->setAlpha( 0.0f ); } )
		.set( offscreen )
		.holdUntil( instructions_start )
		.then<RampTo>( instructionsPosition, 0.5f, ch::EaseOutQuad() )
		.hold( 3.0f )
		.then<RampTo>( offscreen, 0.5f, ch::EaseInQuad() );
}

void AboutPage::hide()
{
	visible = false;
	description->setAlpha( 0.0f );
	nestingButton->setEnabled( false );
	nestingButton->hide( timeline );

	auto offscreen = vec2(instructionsPosition.x, - screenshotInstructions->getSize().y);
	sharedTimeline().append( screenshotInstructions->getPositionAnim() )
		.hold( 3.0f )
		.then<RampTo>( offscreen, 0.5f, ch::EaseInQuad() )
		.finishFn( [this] (Motion<vec2> &m) { screenshotInstructions->setAlpha( 0.0f ); } );
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
	nestingButton->hide( timeline );
	closeButton->setEnabled( true );

	description->setAlpha( 1.0f );
}

void AboutPage::hideAbout()
{
	description->setAlpha( 0.0f );
	closeButton->setEnabled( false );
	nestingButton->setEnabled( true );
}

ch::TimelineOptions AboutPage::showIcon()
{
	nestingButton->show( timeline );

	// Scoped control cancels previous call automatically, ensuring this cue is only fired once.
	auto opt = timeline.cue( [this] { nestingButton->hide( timeline ); }, 3.0f );
	hideCue = opt.getScopedControl();

	return opt;
}
