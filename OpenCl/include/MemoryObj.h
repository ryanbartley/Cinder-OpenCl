//
//  MemoryObj.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#pragma once

#include <OpenCl/OpenCl.h>
#include "Context.h"

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class MemoryObj> MemoryObjRef;
	
class MemoryObj {
public:
	~MemoryObj()
	{
		clReleaseMemObject( mId );
	}
	
	cl_mem				getId() const { return mId; }
	cl_mem_flags		getFlags() const { return mFlags; }
	ContextRef&			getContext() { return mContext; }
	const ContextRef&	getContext() const { return mContext; }
	
protected:
	MemoryObj( const ContextRef& context ) : mContext( context ) {}
	
	cl_mem			mId;
	cl_mem_flags	mFlags;
	ContextRef		mContext;
};
}
}