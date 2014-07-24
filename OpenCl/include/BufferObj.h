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
#include "MemoryObj.h"

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class BufferObj> BufferObjRef;
typedef std::shared_ptr<class CommandQueue> CommandQueueRef;
typedef std::shared_ptr<class Event> EventRef;
typedef void(*BufferDestructorCallback)( cl_mem, void *);
	
//typedef struct _cl_buffer_region {
//	size_t origin;
//	size_t size;
//} cl_buffer_region;

class BufferObj : public boost::noncopyable, public std::enable_shared_from_this<BufferObj>, public MemoryObj {
public:
	static BufferObjRef create( cl_mem_flags flags, size_t size, void *data );
	static BufferObjRef create( const gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
	class SubBuffer : public boost::noncopyable, public std::enable_shared_from_this<SubBuffer>, public MemoryObj {
	public:
		typedef std::shared_ptr<SubBuffer> SubBufferRef;
		
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, size_t origin, size_t size );
		
		~SubBuffer(){}
		
	private:
		SubBuffer( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		
		size_t			mOrigin;
		size_t			mSize;
		BufferObjRef	mParent;
	};
	
	typedef std::shared_ptr<SubBuffer> SubBufferRef;
	
	~BufferObj() {}
	
	size_t			getSize() { return mSize; }
	
	// TODO: Need to decide if these are good options
	void partitionBuffer( size_t origin, size_t size );
	void partitionBuffer( size_t divisor );
	
private:
	BufferObj( cl_mem_flags flags, size_t size, void *data );
	// TODO: Make a directX buffer object for shared context
	BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
	void setReturnEvent( EventRef *returnEvent, cl_event event );
	
	size_t						mSize;
	std::vector<SubBufferRef>	mSubBuffers;
};
	
typedef std::shared_ptr<BufferObj::SubBuffer> SubBufferRef;
	
}}
