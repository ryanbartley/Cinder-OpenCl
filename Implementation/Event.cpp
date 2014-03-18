//
//  Event.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#include "Event.h"

namespace cinder { namespace cl {

EventRef Event::create( cl_event event )
{
	return EventRef( new Event( event ) );
}
	
Event::Event( cl_event event )
: mId( event )
{	
}
	
}}