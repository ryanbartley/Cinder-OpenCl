//
//  CommandQueue.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#include "CommandQueue.h"
#include "Context.h"
#include "Device.h"

namespace cinder { namespace cl {

CommandQueue::CommandQueue( const DeviceRef &device, cl_command_queue_properties properties  )
: mId( nullptr ), mProperties( properties )
{
	cl_int errNum;
	
	mId = clCreateCommandQueue( Context::context()->getId(), device->getId(), properties, &errNum );

	if( errNum != CL_SUCCESS ) {
		std::cerr << "Error: Creating Command Queue - " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
CommandQueueRef CommandQueue::create( const DeviceRef &device, cl_command_queue_properties properties )
{
	return CommandQueueRef( new CommandQueue( device, properties ) );
}
	
CommandQueue::~CommandQueue()
{
	clReleaseCommandQueue(mId);
}
	
}}