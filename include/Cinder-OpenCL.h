//
//  Cinder-OpenCL.h
//  HelloWorld
//
//  Created by ryan bartley on 6/12/15.
//
//

#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

namespace cl {
	using PlatformRef	= std::shared_ptr<::cl::Platform>;
	using BufferGLRef	= std::shared_ptr<::cl::BufferGL>;
	using BufferRef		= std::shared_ptr<::cl::Buffer>;
	using CommandQueueRef = std::shared_ptr<::cl::CommandQueue>;
	using ContextRef	= std::shared_ptr<::cl::Context>;
	using ProgramRef	= std::shared_ptr<::cl::Program>;
	using KernelRef		= std::shared_ptr<::cl::Kernel>;
}

namespace cinder {	
	std::ostream& operator<<( std::ostream &lhs, const cl::Platform &rhs );
	std::ostream& operator<<( std::ostream &lhs, const cl::Device &rhs );
	cl_context_properties* getDefaultSharedGraphicsContextProperties();
}
