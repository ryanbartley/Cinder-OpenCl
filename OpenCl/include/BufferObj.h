//
//  BufferObj.h
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "cinder/gl/BufferObj.h"

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class BufferObj> BufferObjRef;
typedef std::shared_ptr<class CommandQueue> CommandQueueRef;
typedef std::shared_ptr<class Event> EventRef;
	
//typedef struct _cl_buffer_region {
//	size_t origin;
//	size_t size;
//} cl_buffer_region;

class BufferObj : public boost::noncopyable, public std::enable_shared_from_this<BufferObj> {
public:
	static BufferObjRef create( cl_mem_flags flags, size_t size, void *data );
	static BufferObjRef create( const gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
	class SubBuffer : public boost::noncopyable, public std::enable_shared_from_this<SubBuffer> {
	public:
		typedef std::shared_ptr<SubBuffer> SubBufferRef;
		
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, size_t origin, size_t size );
		
		~SubBuffer();
		
	private:
		SubBuffer( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		
		cl_mem			mId;
		cl_mem_flags	mFlags;
		size_t			mOrigin;
		size_t			mSize;
	};
	
	typedef std::shared_ptr<SubBuffer> SubBufferRef;
	
	~BufferObj();
	
	cl_mem			getId() { return  mId; }
	size_t			getSize() { return mSize; }
	cl_mem_flags	getFlags() { return mFlags; }
	
	// TODO: Need to decide if these are good options
	void partitionBuffer( size_t origin, size_t size );
	void partitionBuffer( size_t divisor );
	
	void enqueueWrite( const CommandQueueRef &commandQueue, cl_bool blockWrite, size_t offset, size_t size, void *data, const std::vector<EventRef> *eventWaitList = nullptr, EventRef *writeEvent = nullptr );
	void enqueueRead( const CommandQueueRef &commandQueue, cl_bool blockRead, size_t offset, size_t size, void *data, const std::vector<EventRef> *eventWaitList = nullptr, EventRef *readEvent = nullptr );
	void* enqueueMap( const CommandQueueRef &commandQueue, cl_bool blockMap, cl_map_flags flags, size_t offset, size_t size, const std::vector<EventRef> *eventWaitList = nullptr, EventRef *readEvent = nullptr );
	void enqueueUnMap( const CommandQueueRef &commandQueue, void *mapped_pointer, const std::vector<EventRef> *eventWaitList = nullptr, EventRef *event = nullptr );
	void enqueueCopy( const CommandQueueRef &commandQueue, const BufferObjRef &copyBuffer, size_t copyBufferOffset, size_t thisBufferOffset, size_t size, const std::vector<EventRef> *eventWaitList = nullptr, EventRef *event = nullptr );
private:
	BufferObj( cl_mem_flags flags, size_t size, void *data );
	// TODO: Make a directX buffer object for shared context
	BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
	void setReturnEvent( EventRef *returnEvent, cl_event event );
	
	cl_mem						mId;
	size_t						mSize;
	cl_mem_flags				mFlags;
	std::vector<SubBufferRef>	mSubBuffers;
};
	
}}
