//
//  Program.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#include "Program.h"
#include "Context.h"

namespace cinder { namespace cl {

	Program::Program( const DataSourceRef &dataSource )
	{
		cl_int errNum;
		std::string program;
		Buffer buffer( dataSource );
		program.resize( buffer.getDataSize() + 1 );
		memcpy( (void*)program.data(), buffer.getData(), buffer.getDataSize() );
		program[buffer.getDataSize()] = 0;
		
		mId = clCreateProgramWithSource( Context::context()->getId(), 0, (const char** )program.c_str(), nullptr, &errNum );
		
		if( mId == NULL ) {
			std::cerr << "Failed to create CL program from source" << std::endl;
			exit(EXIT_FAILURE);
		}
		
		errNum = clBuildProgram( mId, 0, NULL, NULL, NULL, NULL );
		
		if (errNum != CL_SUCCESS)
		{
			// Determine the reason for the error
			char buildLog[16384];
			clGetProgramBuildInfo(mId, nullptr, CL_PROGRAM_BUILD_LOG,
								  sizeof(buildLog), buildLog, NULL);
			std::cerr << "Error in kernel: " << std::endl;
			std::cerr << buildLog;
			clReleaseProgram(mId);
			exit(EXIT_FAILURE);
		}
	}
	
	Program::~Program()
	{
		clRetainProgram(mId);
	}
	
}}