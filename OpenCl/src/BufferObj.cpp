//
//  BufferObj.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#include "BufferObj.h"
#include "Context.h"
#include "CommandQueue.h"
#include "Event.h"

namespace cinder { namespace cl {
	
BufferObj::SubBuffer::SubBuffer( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo )
: mId( nullptr ), mFlags( flags ), mSize( bufferCreateInfo->size ), mOrigin( bufferCreateInfo->origin )
{
	cl_int errNum;
	mId = clCreateSubBuffer( buffer->getId(), mFlags, CL_BUFFER_CREATE_TYPE_REGION, bufferCreateInfo, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Creating SubBuffer - " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
	if( mId == NULL ) {
		std::cout << "ERROR: Creating SubBuffer Id is null" << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
BufferObj::SubBufferRef BufferObj::SubBuffer::create( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo )
{
	return BufferObj::SubBufferRef( new BufferObj::SubBuffer( buffer, flags, bufferCreateInfo ) );
}
	
BufferObj::SubBufferRef BufferObj::SubBuffer::create( const BufferObjRef &buffer, cl_mem_flags flags, size_t origin, size_t size )
{
	cl_buffer_region bufferOrigin;
	bufferOrigin.origin = origin;
	bufferOrigin.size = size;
	return BufferObj::SubBufferRef( new BufferObj::SubBuffer( buffer, flags, &bufferOrigin ) );
}
	
BufferObj::SubBuffer::~SubBuffer()
{
	clReleaseMemObject(mId);
}

BufferObj::BufferObj( cl_mem_flags flags, size_t size, void *data )
: mId( nullptr ), mSize( size ), mFlags( flags )
{
	cl_int errNum;
	
	mId = clCreateBuffer( Context::context()->getId(), mFlags, mSize, data, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Creating Buffer - " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
	if( mId == NULL ) {
		std::cout << "ERROR: Creating Buffer Id is null" << std::endl;
		exit(EXIT_FAILURE);
	}
	
}
	
BufferObj::~BufferObj()
{
	clReleaseMemObject( mId );
}
	
BufferObjRef BufferObj::create( cl_mem_flags flags, size_t size, void *data )
{
	return BufferObjRef( new BufferObj( flags, size, data ) );
}
	
void BufferObj::partitionBuffer( size_t origin, size_t size )
{
	
}
	
void BufferObj::partitionBuffer( size_t divisor )
{
	
}
	
void BufferObj::enqueueWrite( const CommandQueueRef &commandQueue, cl_bool blockWrite, size_t offset, size_t size, void *data, const std::vector<EventRef> *eventWaitList, EventRef *writeEvent )
{
	cl_int errNum;
	
	if( eventWaitList ) {
		if( writeEvent ) {
			cl_event event;
			
			auto eventIdList = EventList::createEventIdList( *eventWaitList );
				
			errNum = clEnqueueWriteBuffer( commandQueue->getId(), mId, blockWrite, offset, size, data, eventIdList.size(), eventIdList.data(), &event );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
			setReturnEvent( writeEvent, event );
			
		}
		else {
			auto eventIdList = EventList::createEventIdList( *eventWaitList );
			
			errNum = clEnqueueWriteBuffer( commandQueue->getId(), mId, blockWrite, offset, size, data, eventIdList.size(), eventIdList.data(), nullptr );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	else {
		if( writeEvent ) {
			cl_event event;
			
			errNum = clEnqueueWriteBuffer( commandQueue->getId(), mId, blockWrite, offset, size, data, 0, nullptr, &event );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
			setReturnEvent( writeEvent, event );
		}
		else {
			
			errNum = clEnqueueWriteBuffer( commandQueue->getId(), mId, blockWrite, offset, size, data, 0, nullptr, nullptr );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}
}
	
void BufferObj::enqueueRead( const CommandQueueRef &commandQueue, cl_bool blockRead, size_t offset, size_t size, void *data, const std::vector<EventRef> *eventWaitList, EventRef *readEvent )
{
	cl_int errNum;
	
	if( eventWaitList ) {
		if( readEvent ) {
			cl_event event;
			
			auto eventIdList = EventList::createEventIdList( *eventWaitList );
			
			errNum = clEnqueueReadBuffer( commandQueue->getId(), mId, blockRead, offset, size, data, eventIdList.size(), eventIdList.data(), &event );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
			setReturnEvent( readEvent, event );
			
		}
		else {
			auto eventIdList = EventList::createEventIdList( *eventWaitList );
			
			errNum = clEnqueueReadBuffer( commandQueue->getId(), mId, blockRead, offset, size, data, eventIdList.size(), eventIdList.data(), nullptr );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	else {
		if( readEvent ) {
			cl_event event;
			
			errNum = clEnqueueReadBuffer( commandQueue->getId(), mId, blockRead, offset, size, data, 0, nullptr, &event );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
			setReturnEvent( readEvent, event );
		}
		else {
			
			errNum = clEnqueueReadBuffer( commandQueue->getId(), mId, blockRead, offset, size, data, 0, nullptr, nullptr );
			
			if( errNum != CL_SUCCESS ) {
				std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}

}
	
void BufferObj::setReturnEvent( EventRef *returnEvent, cl_event event )
{
	// TODO: This is so dirty have to clean up
	if( returnEvent != nullptr && event != nullptr ) {
		returnEvent->reset( SysEvent::create( event ).get() );
	}
}
	
BufferObj::BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags )
{
	//TODO: Check whether this is a shared OpenGl context
//	cl_int errcode;
//	
//	mId = clCreateFromGLBuffer( Context::context()->mContext, flags, glBuffer->getId(), &errcode );
//	
//	if( errcode ) {
//		std::cout << "ERROR: " << errcode << std::endl;
//	}
}
	
}}