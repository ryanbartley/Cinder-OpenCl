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
: MemoryObj( buffer->getContext() ), mSize( bufferCreateInfo->size ), mOrigin( bufferCreateInfo->origin ), mParent( buffer )
{
	cl_int errNum;
	mId = clCreateSubBuffer( mParent->getId(), mFlags, CL_BUFFER_CREATE_TYPE_REGION, bufferCreateInfo, &errNum );
	
	setDestructorCallback( MemoryObj::destructionCallback, this );
	
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

BufferObj::BufferObj( cl_mem_flags flags, size_t size, void *data )
: MemoryObj( Context::context() ), mSize( size )
{
	cl_int errNum;
	
	mFlags = flags;
	mId = clCreateBuffer( getContext()->getId(), mFlags, mSize, data, &errNum );
	
	// Todo: Throw
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Creating Buffer - " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
	if( mId == NULL ) {
		std::cout << "ERROR: Creating Buffer Id is null" << std::endl;
		exit(EXIT_FAILURE);
	}
	
}
	
BufferObj::BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags )
: MemoryObj( Context::context() )
{
	cl_int errcode = CL_SUCCESS;
	mFlags = flags;
	
	//TODO: Throw ContextGl Error
	if ( getContext()->isGlShared() ) {
		mId = clCreateFromGLBuffer( getContext()->getId(), mFlags, glBuffer->getId(), &errcode );
	}
	else {
		std::cout << "ERROR: Context is not GL" << std::endl;
	}
	
	// TODO: Throw Buffer error
	if( errcode ) {
		std::cout << "ERROR: " << errcode << std::endl;
	}
}
	
BufferObjRef BufferObj::create( cl_mem_flags flags, size_t size, void *data )
{
	return BufferObjRef( new BufferObj( flags, size, data ) );
}
	
BufferObjRef BufferObj::create( const gl::BufferObjRef &glBuffer, cl_mem_flags flags )
{
	return BufferObjRef( new BufferObj( glBuffer, flags ) );
}
	
void BufferObj::partitionBuffer( size_t origin, size_t size )
{
	
}
	
void BufferObj::partitionBuffer( size_t divisor )
{
	
}
	
}}