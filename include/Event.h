//
//  Event.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#pragma once

#include <OpenCl/OpenCl.h>

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class Event> EventRef;
typedef std::shared_ptr<class SysEvent> SysEventRef;
typedef std::shared_ptr<class UserEvent> UserEventRef;
typedef std::shared_ptr<class Context> ContextRef;
typedef void(CL_CALLBACK *EventCallback)(cl_event event, cl_int event_command_exec_status, void *user_data);
	
enum EventType {
	UNDEFINED_EVENT,
	SYS_EVENT,
	USER_EVENT
};

class Event {
public:
	//! Constructor for Event as wrapper
	explicit Event( cl_event event, EventType type = SYS_EVENT );
	//! Default constructor for a sys_event, sets mId = nullptr and mType = SYS_EVENT
	explicit Event();
	//! This is to use the c functions to initialize a retained event.
	operator cl_event*() { return &mId; }
	//! Constructor for UserEvent
	explicit Event( const ContextRef &context );
	//! copy assignment operator
	Event& operator=( const Event &rhs );
	//! move assignment operator
	Event& operator=( Event &&rhs );
	//! move constructor
	Event( Event &&rhs );
	//! copy constructor
	Event( const Event &rhs );
	
	~Event();
	
	//! Returns the opencl event object contained within.
	cl_event getId() const { return mId; }
	//! Returns the type inside.
	EventType getType() const { return mType; }
	
	//! Sets Event status to status only if \a mType is USER_EVENT. Normal options are CL_COMPLETE, CL_RUNNING, CL_SUBMITTED or CL_QUEUED
	void setUserEventStatus( cl_int status );
	//! Sets the Events Callback function and user_data
	void setCompletedCallback( EventCallback pFunc, void *userData = nullptr );
	
	
	static EventRef create( const Event &event );
	
protected:
	
	cl_event	mId;
	EventType	mType;
};

// I can see that this will be a very used asset maybe
class EventList {
public:
	EventList( int numEvents = 0 ) : mList(numEvents) {}
	EventList( const std::vector<Event> &list ) : mList(list.begin(), list.end()) {}
	EventList& operator=( const EventList &eventList )
	{
		mList = eventList.mList;
		return *this;
	}
	EventList& operator=( EventList &&eventList )
	{
		mList = std::move( eventList.mList );
		return *this;
	}
	EventList( const EventList &eventList ) : mList(eventList.mList) {}
	EventList( EventList &&eventList ) : mList( std::move(eventList.mList) ) {}
	
	inline void push_back( const Event event ) { mList.push_back( event ); }
	inline void pop_back() { mList.pop_back(); }
	
	inline std::list<Event>& getList() { return mList; }
	
	inline size_t size() { return mList.size(); }
	
	inline void clear() { mList.clear(); }
	
	inline std::vector<cl_event> getEventIdList() const
	{
		std::vector<cl_event> eventIdList(mList.size());
		std::transform( mList.begin(), mList.end(), eventIdList.begin(),
					   [&]( const Event &event ) {
						   return event.getId();
					   });
		std::cout << "mList Size " << mList.size() << " eventIdList size: " << eventIdList.size() << std::endl;
		return eventIdList;
	}
	
private:
	std::list<Event> mList;
};
	
}}
