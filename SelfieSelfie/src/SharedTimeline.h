//
//  SharedTimeline.h
//
//  Created by Soso Limited on 8/24/15.
//
//

#pragma once
#include "choreograph/Choreograph.h"

namespace soso {

inline choreograph::Timeline& sharedTimeline()
{
	static ch::Timeline sTimeline;
	return sTimeline;
}

} // namespace soso
