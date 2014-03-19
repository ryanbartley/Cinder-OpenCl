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

class Event : public boost::noncopyable, public std::enable_shared_from_this<Event> {
public:
	static EventRef create( cl_event event );
	cl_event getId() { return mId; }
	
private:
	Event( cl_event event );
	
	cl_event mId;
};

// I can see that this will be a very used asset maybe
class EventList {
public:
	EventList();
	EventList( const std::vector<EventRef> &list );
	
	inline void push_back( const EventRef event ) { mList.push_back( event ); }
	inline void pop_back() { mList.pop_back(); }
	inline EventRef* data() { return mList.data(); }
	
	inline size_t size() { return mList.size(); }
	
	inline void clear() { mList.clear(); }
	
	inline std::vector<cl_event> getEventIdList()
	{
		std::vector<cl_event> eventIdList;
		for( auto eventIdIt = mList.begin(); eventIdIt != mList.end(); ++eventIdIt ) {
			eventIdList.push_back( (*eventIdIt)->getId() );
		}
		return eventIdList;
	}
	inline static std::vector<cl_event> createEventIdList( const std::vector<EventRef> &eventList )
	{
		std::vector<cl_event> eventIdList;
		for( auto eventIdIt = eventList.begin(); eventIdIt != eventList.end(); ++eventIdIt ) {
			eventIdList.push_back( (*eventIdIt)->getId() );
		}
		return eventIdList;
	}
	
private:
	std::vector<EventRef> mList;
};
	
}}
