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
	clRetainEvent( mId );
}
	
SysEventRef SysEvent::create( cl_event event )
{
	return SysEventRef( new SysEvent( event ) );
}
	
SysEvent& SysEvent::operator=( const SysEvent &rhs )
{
	if (this != &rhs) {
		mId = rhs.mId;
		clRetainEvent( mId );
		mType = rhs.mType;
	}
	return *this;
}
	
SysEvent& SysEvent::operator=( SysEvent &&rhs )
{
	if (this != &rhs) {
		
	}
	return *this;
}

SysEvent::SysEvent( SysEvent &&rhs )
: Event( std::move(rhs.mType) )
{
	mId = rhs.mId;
	clRetainEvent( mId );
	rhs.mId = 0;
	rhs.mType = (EventType)-1;
}

SysEvent::SysEvent( const SysEvent &rhs )
: Event( std::move(rhs.mType) )
{

}

SysEvent::~SysEvent(){}
	
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