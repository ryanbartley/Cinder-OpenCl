//
//  Event.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#include "Event.h"
#include "Context.h"

namespace cinder { namespace cl {
	
Event::Event( EventType type )
: mType( type )
{	
}
	
Event::~Event()
{
	clReleaseEvent( mId );
}
	
void Event::setCallback( EventCallback pFunc, void *userData )
{
	cl_int errNum;
	errNum = clSetEventCallback( mId, CL_COMPLETE, pFunc, userData );
}
	
SysEvent::SysEvent( cl_event event )
: Event( SYS_EVENT )
{
	mId = event;
}
	
SysEventRef SysEvent::create( cl_event event )
{
	return SysEventRef( new SysEvent( event ) );
}
	
UserEvent::UserEvent( const ContextRef &context )
: Event( USER_EVENT )
{
	cl_int errNum;
	mId = clCreateUserEvent( context->getId(), &errNum );
}
	
void UserEvent::setStatus( cl_int executionStatus )
{
	cl_int errNum;
	errNum = clSetUserEventStatus( mId, executionStatus );
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Set user status failed with " << errNum << std::endl;
	}
}
	
}}