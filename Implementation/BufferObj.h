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

class BufferObj {
public:
	static BufferObjRef create( cl_mem_flags flags, size_t size, void *data );
	
	~BufferObj();
	
	cl_mem			getId() { return  mId; }
	size_t			getSize() { return mSize; }
	cl_mem_flags	getFlags() { return mFlags; }
	
private:
	BufferObj( cl_mem_flags flags, size_t size, void *data );
	BufferObj( const gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
	cl_mem			mId;
	size_t			mSize;
	cl_mem_flags	mFlags;
};
	
}}
