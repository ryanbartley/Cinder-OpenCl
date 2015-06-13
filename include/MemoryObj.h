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

namespace cl {
	
typedef std::shared_ptr<class MemoryObj> MemoryObjRef;
typedef void(*MemObjDestructorCallback)( cl_mem, void *);
	
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
	
	void setDestructorCallback( MemObjDestructorCallback callback, void *userData = nullptr );
	
protected:
	MemoryObj( const ContextRef& context );
	
	static void CL_CALLBACK destructionCallback( cl_mem destructedMem, void * userData );
	
	cl_mem			mId;
	cl_mem_flags	mFlags;
	ContextRef		mContext;
};

} // namespace cl
