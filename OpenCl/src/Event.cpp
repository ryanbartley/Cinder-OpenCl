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
		mId = rhs.mId;
		mType = rhs.mType;
		rhs.mId = nullptr;
		rhs.mType = (EventType)-1;
	}
	return *this;
}

SysEvent::SysEvent( SysEvent &&rhs )
: Event( rhs.mType )
{
	mId = rhs.mId;
	rhs.mId = nullptr;
	rhs.mType = (EventType)-1;
}

SysEvent::SysEvent( const SysEvent &rhs )
: Event( rhs.mType )
{
	mId = rhs.mId;
	clRetainEvent( mId );
}

SysEvent::~SysEvent()
{
}
	
UserEvent::UserEvent( const ContextRef &context )
: Event( USER_EVENT )
{
	cl_int errNum;
	mId = clCreateUserEvent( context->getId(), &errNum );
}
	
UserEvent& UserEvent::operator=( const UserEvent &rhs )
{
	if (this != &rhs) {
		mId = rhs.mId;
		clRetainEvent( mId );
		mType = rhs.mType;
	}
	return *this;
}

UserEvent& UserEvent::operator=( UserEvent &&rhs )
{
	if (this != &rhs) {
		mId = rhs.mId;
		mType = rhs.mType;
		rhs.mId = nullptr;
		rhs.mType = (EventType)-1;
	}
	return *this;
}

UserEvent::UserEvent( UserEvent &&rhs )
: Event( rhs.mType )
{
	mId = rhs.mId;
	rhs.mId = nullptr;
	rhs.mType = (EventType)-1;
}

UserEvent::UserEvent( const UserEvent &rhs )
: Event( rhs.mType )
{
	mId = rhs.mId;
	clRetainEvent( mId );
}

UserEvent::~UserEvent()
{
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