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
	NestingButton( const ci::DataSourceRef &iIcon, const ci::DataSourceRef &iBacking, const std::function<void ()> &iCallback );

	void draw() const;
	void show( ci::Timeline &iTimeline );
	void hide( ci::Timeline &iTimeline );

	void setEnabled( bool iEnabled ) { openButton->setEnabled( iEnabled ); }
	bool isHidden() const { return hidden; }
private:
	std::unique_ptr<Image>			icon;
	std::unique_ptr<Image>			iconBacking;
	std::unique_ptr<TouchArea>	openButton;
	ci::Anim<ci::vec2>					position;
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
	ci::TimelineRef	timeline = ci::Timeline::create();
	std::unique_ptr<Image>					description;
	std::unique_ptr<Image>					screenshotInstructions;
	std::unique_ptr<TouchArea>			closeButton;
	std::unique_ptr<NestingButton>	nestingButton;
	ci::Timer												timer;
	ci::CueRef											hideCue = nullptr;

	void handleIconClick();
	void showAbout();
	void hideAbout();
	void showIcon();

	bool visible = false;
};

} // namespace soso
