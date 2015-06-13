//
//  MemoryObj.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#include "MemoryObj.h"
#include "Platform.h"
#include "ConstantConversion.h"

namespace cl {

MemoryObj::MemoryObj( const ContextRef& context )
: mContext( context )
{
}
	
void MemoryObj::setDestructorCallback( MemObjDestructorCallback callback, void *userData )
{
	cl_int errNum = clSetMemObjectDestructorCallback( mId, callback, userData );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: In setDestructorCallback " << getErrorString( errNum ) << std::endl;
	}
}
	
void MemoryObj::destructionCallback( cl_mem destructedMem, void *userData )
{
	std::cout << "A memory object has been destroyed" << std::endl;
}

} // namespace cl