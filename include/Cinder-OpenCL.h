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

namespace cinder {	
	std::ostream& operator<<( std::ostream &lhs, const cl::Platform &rhs );
	std::ostream& operator<<( std::ostream &lhs, const cl::Device &rhs );
	cl_context_properties* getDefaultSharedGraphicsContextProperties();
}
