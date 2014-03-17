//
//  BufferObj.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#include "BufferObj.h"
#include "Context.h"

namespace cinder { namespace cl {

BufferObj::BufferObj( cl_mem_flags flags, size_t size, void *data )
: mSize( size ), mFlags( flags )
{
	cl_int errNum;
	
	mId = clCreateBuffer( Context::context()->getId(), mFlags, mSize, data, &errNum );
	
	if( errNum ) {
		std::cout << "ERROR: Creating Buffer - " << errNum << std::endl;
	}
	if( mId == NULL ) {
		std::cout << "ERROR: Creating Buffer Id is null" << std::endl;
	}
}
	
BufferObjRef BufferObj::create( cl_mem_flags flags, size_t size, void *data )
{
	return BufferObjRef( new BufferObj( flags, size, data ) );
}
	
BufferObj::BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags )
{
//	cl_int errcode;
//	
//	mId = clCreateFromGLBuffer( Context::context()->mContext, flags, glBuffer->getId(), &errcode );
//	
//	if( errcode ) {
//		std::cout << "ERROR: " << errcode << std::endl;
//	}
}
	
}}