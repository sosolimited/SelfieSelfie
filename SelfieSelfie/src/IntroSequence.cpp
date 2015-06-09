//
//  IntroSequence.cpp
//
//  Created by Soso Limited on 6/9/15.
//
//

#include "IntroSequence.h"

using namespace soso;

void IntroSequence::setup( const ci::fs::path &iImageBasePath )
{
	timeline->clear();
	timeline->reset();

//	timeline->appendTo(<#Anim<T> *target#>, <#T endValue#>, <#float duration#>);
}

void IntroSequence::handleFinish()
{
	if( finishFn ) {
		finishFn();
	}
}
