//
//  TouchArea.h
//
//  Created by Soso Limited on 6/10/15.
//
//

#pragma once

namespace soso {

///
/// A rectangle that calls a function on tap.
/// No coordination between touch areas.
///
class TouchArea
{
public:
	~TouchArea()
	{
		disconnect();
	}

	static std::shared_ptr<TouchArea> create( const ci::Rectf &iBounds, const std::function<void ()> &iCallback );

	void setEnabled( bool iEnabled ) { enabled = iEnabled; }
	bool isEnabled() const { return enabled; }

private:
	ci::Rectf								bounds;
	std::function<void ()>	callback;
	uint32_t								trackedTouch = 0;
	bool										wasInside = false;
	bool										enabled = true;
	ci::signals::Connection	touchBeginConnection, touchEndConnection;

	void touchBegin( const ci::app::TouchEvent &iEvent );
	void touchEnd( const ci::app::TouchEvent &iEvent );
	void connect();
	void disconnect();

	TouchArea( const ci::Rectf &iBounds, const std::function<void ()> &iCallback )
	: bounds( iBounds ),
		callback( iCallback )
	{
		connect();
	}
};

} // namespace soso
