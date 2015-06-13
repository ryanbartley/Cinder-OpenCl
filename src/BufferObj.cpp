//
//  BufferObj.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#include "cinder/Log.h"

#include "BufferObj.h"
#include "Context.h"
#include "CommandQueue.h"
#include "Event.h"
#include "ConstantConversion.h"

namespace cl {
	
BufferObj::SubBuffer::SubBuffer( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo )
: MemoryObj( buffer->getContext() ), mSize( bufferCreateInfo->size ), mOrigin( bufferCreateInfo->origin ), mParent( buffer )
{
	cl_int errNum;
	mId = clCreateSubBuffer( mParent->getId(), mFlags, CL_BUFFER_CREATE_TYPE_REGION, bufferCreateInfo, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		CI_LOG_E( "Creating SubBuffer - " << getErrorString( errNum ) );
	}
	if( ! mId ) {
		CI_LOG_E( "Creating SubBuffer, Id is null" );
	}
	
	setDestructorCallback( MemoryObj::destructionCallback, this );
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

BufferObj::BufferObj( cl_mem_flags flags, size_t size, void *data )
: MemoryObj( Context::context() ), mSize( size )
{
	cl_int errNum;
	
	mFlags = flags;
	mId = clCreateBuffer( getContext()->getId(), mFlags, mSize, data, &errNum );
	
	// Todo: Throw
	if( errNum != CL_SUCCESS ) {
		// Change to throw.
		CI_LOG_E( "Creating Buffer - " << getErrorString( errNum ) );
	}
	if( mId == NULL ) {
		CI_LOG_E( "Creating Buffer Id is null" );
	}
	
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
	
}