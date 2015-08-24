//
//  AboutPage.h
//
//  Created by Soso Limited on 6/10/15.
//
//

#pragma once
#include "Image.h"
#include "TouchArea.h"
#include "cinder/Timer.h"

namespace soso {

class NestingButton
{
public:
	NestingButton( std::unique_ptr<Image> &&iIcon, const std::function<void ()> &iCallback, const ci::vec2 &iOpenPosition );

	void draw() const;
	void show( ch::Timeline &iTimeline );
	void hide( ch::Timeline &iTimeline );

	void setEnabled( bool iEnabled ) { touchArea->setEnabled( iEnabled ); }
	bool isHidden() const { return hidden; }

private:
	std::unique_ptr<Image>			icon;
	std::unique_ptr<TouchArea>	touchArea;
	ci::vec2										openPosition;
	ci::vec2										closedPosition;

	bool hidden = false;
};

class AboutPage
{
public:
	void setup( const ci::fs::path &iDirectory );
	void update();
	void draw();

	void show();
	void hide();

private:
	ch::Timeline										timeline;
	std::unique_ptr<Image>					description;
	std::unique_ptr<Image>					screenshotInstructions;
	ci::vec2												instructionsPosition;
	std::unique_ptr<TouchArea>			closeButton;
	std::unique_ptr<NestingButton>	nestingButton;
	ci::Timer												timer;
	ch::ScopedCancelRef							hideCue;

	void handleIconClick();
	void showAbout();
	void hideAbout();
	ch::TimelineOptions showIcon();

	bool visible = false;
};

} // namespace soso
