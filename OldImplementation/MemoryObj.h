//
//  MemoryObj.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#pragma once

#include <OpenCl/OpenCl.h>

// Possible Base class for BufferObj and Image, Need more research to figure out if it is needed
namespace cinder { namespace cl {
	
	class MemoryObj {
	public:
		~MemoryObj();
		
	protected:
		MemoryObj();
		
		cl_mem			mId;
		cl_mem_flags	mFlags;
	};
}
}