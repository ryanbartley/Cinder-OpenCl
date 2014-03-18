//
//  CommandQueue.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class CommandQueue> CommandQueueRef;
typedef std::shared_ptr<class Device> DeviceRef;
typedef std::shared_ptr<class Context> ContextRef;

class CommandQueue : public boost::noncopyable, public std::enable_shared_from_this<CommandQueue> {
public:
	static CommandQueueRef create( const DeviceRef &device, cl_command_queue_properties properties = 0 );
	
	~CommandQueue();
	
	cl_command_queue getId() { return mId; }
private:
	CommandQueue( const DeviceRef &device, cl_command_queue_properties properties );
	
	cl_command_queue			mId;
	cl_command_queue_properties mProperties;
};
	
}}
