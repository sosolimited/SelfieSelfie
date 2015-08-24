//
//  TapHandler.cpp
//
//  Created by Soso Limited on 8/21/15.
//
//

#include "TapHandler.h"
#include "cinder/app/App.h"

using namespace soso;
using namespace cinder;

TapHandler::TapHandler()
{
	connections[0] = app::getWindow()->getSignalTouchesBegan().connect( [this] (const app::TouchEvent &event) {
		touchId = event.getTouches().back().getId();
		touchTimer.start();
	});

	connections[1] = app::getWindow()->getSignalTouchesEnded().connect( [this] (const app::TouchEvent &event) {
		if (touchId == event.getTouches().back().getId())
		{
			touchTimer.stop();
			if (touchTimer.getSeconds() < touchDuration) {
				tapped.emit();
			}
		}
	});
}
