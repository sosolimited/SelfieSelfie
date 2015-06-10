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
	std::unique_ptr<Image>			description;
	std::unique_ptr<Image>			icon;
	std::unique_ptr<Image>			iconBacking;
	std::shared_ptr<TouchArea>	openButton;
	std::shared_ptr<TouchArea>	closeButton;
	ci::Timer										timer;

	void showAbout();
	void showIcon();

	bool visible = false;
};

} // namespace soso
