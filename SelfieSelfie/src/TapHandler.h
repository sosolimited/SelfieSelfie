//
//  TapHandler.h
//
//  Created by Soso Limited on 8/21/15.
//
//

#pragma once

namespace soso {

/// Handles a screen tap gesture
class TapHandler
{
public:
	TapHandler();

	/// Returns the signal that is emitted when a tap occurs.
	ci::signals::Signal<void ()>& getSignalTapped() { return tapped; }

private:
	std::array<ci::signals::ScopedConnection, 2>	connections;
	ci::signals::Signal<void ()>									tapped;
	uint32_t																			touchId = 0;
	ci::Timer																			touchTimer;
	float																					touchDuration = 0.16f;
};

} // namespace soso
