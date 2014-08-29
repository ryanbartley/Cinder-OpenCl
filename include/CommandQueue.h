//
//  CommandQueue.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#pragma once

#include <OpenCl/opencl.h>
#include "Event.h"
#include "Program.h"
#include "BufferObj.h"
#include "Platform.h"

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
	
	//! A synchronization point that ensures that all queued commands in command_queue have finished execution before the next batch of commands can begin execution.
	void barrier() { clEnqueueBarrier( mId ); }
	//! Enqueues a marker command which waits for either a \a list of events to complete, or if the list is empty it waits for all commands previously enqueued in command_queue to complete before it completes.
	void barrierWithWaitlist( const EventList &list = EventList(), Event *event = nullptr );
	//! Enqueues a marker command to command_queue. The marker command returns an event, through \a event which can be used to queue a wait on this marker event i.e. wait for all commands queued before the marker command to complete.
	void marker( Event *event );
	//! All commands after this call will wait until events in \a list have completed.
	void waitForEvents( const EventList &list );
	//! Finishes all commands to the point that this is called.
	void finish() { clFinish(mId); }
	
	//! Acquires the MemoryObjs, which have to be linked to GL objects. Waits for events inside \a waitEvents, if present and places an event inside \a returnEvent, if present.
	void acquireGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents = EventList(), Event *returnEvent = nullptr );
	//! Releases the MemoryObjs, which have to be linked to GL objects. Waits for events inside \a waitEvents, if present and places an event inside \a returnEvent, if present.
	void releaseGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents = EventList(), Event *returnEvent = nullptr );
	
	//! Writes to \a buffer \a size from \a offset into \a buffer from \a data, which is host pointer. \a blockWrite allows the write to block or not while writing takes place. \a this operation will wait until all events in \a eventList have completed. returnEvent will hold an event after return signaling that this operation has completed.
	void write( const BufferObjRef &buffer, cl_bool blockWrite, size_t offset, size_t size, void *data, const EventList &eventWaitList = EventList(), Event *returnEvent = nullptr );
	//! Writes from \a buffer \a size from \a offset into \a buffer to \a data, which is host pointer. \a blockRead allows the read to block or not while writing takes place. \a this operation will wait until all events in \a eventList have completed. returnEvent will hold an event after return signaling that this operation has completed.
	void read( const BufferObjRef &buffer, cl_bool blockRead, size_t offset, size_t size, void *data, const EventList &eventWaitList = EventList(), Event *returnEvent = nullptr );
	
	void fill(const BufferObjRef &buffer, void* pattern, size_t patternSize, size_t offset, size_t size, const EventList &list = EventList(), Event *returnEvent = nullptr );
	
	template<typename T>
	void fill(const BufferObjRef &buffer, T* pattern, size_t offset = 0, size_t size = -1, const EventList &list = EventList(), Event *returnEvent = nullptr );
	
	void copy( const BufferObjRef &srcBuffer, const BufferObjRef &dstBuffer, size_t srcOffset, size_t dstOffset, size_t size, const EventList &list = EventList(), Event *returnEvent = nullptr );
	
	void* map( const BufferObjRef &buffer, cl_bool blockMap, cl_map_flags mapFlags, size_t offset = 0, size_t size = -1, const EventList &list = EventList(), Event *returnEvent = nullptr );
	
	void unmap( const MemoryObjRef &memObj, void* mappedPointer, const EventList &list = EventList(), Event *event = nullptr );
	
	//! Runs a kernel
	void NDRangeKernel(	const KernelRef &kernel,
					   cl_uint work_dim,
					   size_t *global_work_offset,
					   size_t *global_work_size,
					   size_t *local_work_size,
					   const EventList &waitList = EventList(),
					   Event *returnEvent = nullptr );
	void task( const KernelRef &kernel, const EventList &waitList = EventList(), Event *returnEvent = nullptr );
	
	//! Returns the reference of the device associated with this Command Queue
	DeviceRef& getDevice() { return mDevice; }
	//! Returns the const reference of the device associated with this Command Queue
	const DeviceRef& getDevice() const { return mDevice; }
	//! Returns the reference of the context associated with this Command Queue
	ContextRef& getContext() { return mContext; }
	//! Returns the const reference of the context associated with this Command Queue
	const ContextRef& getContext() const { return mContext; }
	
	void check( const char * something );
	
private:
	CommandQueue( const DeviceRef &device, cl_command_queue_properties properties );
	
	cl_command_queue			mId;
	cl_command_queue_properties mProperties;
	DeviceRef					mDevice;
	ContextRef					mContext;
};
	
template<typename T>
void CommandQueue::fill(const BufferObjRef &buffer, T* pattern, size_t offset, size_t size, const EventList &list, Event *returnEvent )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	auto amountToWrite = size == -1 ? (buffer->getSize() - offset) : size;
	
	errNum = clEnqueueFillBuffer( mId, buffer->getId(), pattern, sizeof(T), offset, amountToWrite, waitList.size(), waitList.data(), returnEvent ? (*returnEvent) : nullptr );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << Platform::getClErrorString( errNum ) << std::endl;
	}
}
	
}}
