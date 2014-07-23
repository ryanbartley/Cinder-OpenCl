//
//  CommandQueue.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "Event.h"
#include "Program.h"

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class CommandQueue> CommandQueueRef;
typedef std::shared_ptr<class Device> DeviceRef;
typedef std::shared_ptr<class Context> ContextRef;
typedef std::shared_ptr<class BufferObj> BufferObjRef;
class Event;
typedef std::shared_ptr<class EventList> EventListRef;
typedef std::shared_ptr<class MemoryObj> MemoryObjRef;

class CommandQueue : public boost::noncopyable, public std::enable_shared_from_this<CommandQueue> {
public:
	
	static CommandQueueRef create( const DeviceRef &device, cl_command_queue_properties properties = 0 );
	
	~CommandQueue();
	
	cl_command_queue getId() { return mId; }
	
	void barrier() { clEnqueueBarrier( mId ); }
	void marker( const Event &event );
	void waitForEvents( const EventListRef &list );
	void finish() { clFinish(mId); }
	
	void acquireGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents = EventList(), Event *returnEvent = nullptr );
	void releaseGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents = EventList(), Event *returnEvent = nullptr );
	
	//! Writes to \a buffer \a size from \a offset into \a buffer from \a data, which is host pointer. \a blockWrite allows the write to block or not while writing takes place. \a this operation will wait until all events in \a eventList have completed. returnEvent will hold an event after return signaling that this operation has completed.
	void write( const BufferObjRef &buffer, cl_bool blockWrite, size_t offset, size_t size, void *data, const EventList &eventWaitList = EventList(), Event *returnEvent = nullptr );
	//! Writes from \a buffer \a size from \a offset into \a buffer to \a data, which is host pointer. \a blockRead allows the read to block or not while writing takes place. \a this operation will wait until all events in \a eventList have completed. returnEvent will hold an event after return signaling that this operation has completed.
	void read( const BufferObjRef &buffer, cl_bool blockRead, size_t offset, size_t size, void *data, const EventList &eventWaitList = EventList(), Event *returnEvent = nullptr );
	
	void NDRangeKernel(	const KernelRef &kernel,
					   cl_uint work_dim,
					   size_t *global_work_offset,
					   size_t *global_work_size,
					   size_t *local_work_size,
					   const EventList &waitList = EventList(),
					   Event *returnEvent = nullptr );
	
	
	DeviceRef& getDevice() { return mDevice; }
	const DeviceRef& getDevice() const { return mDevice; }
	ContextRef& getContext() { return mContext; }
	const ContextRef& getContext() const { return mContext; }
	
private:
	CommandQueue( const DeviceRef &device, cl_command_queue_properties properties );
	
	cl_command_queue			mId;
	cl_command_queue_properties mProperties;
	DeviceRef					mDevice;
	ContextRef					mContext;
};
	
}}
