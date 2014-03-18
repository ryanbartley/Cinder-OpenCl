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

class CommandQueue : public boost::noncopyable, public std::enable_shared_from_this<CommandQueue> {
public:
	
	cl_command_queue getId() { return mId; }
private:

	cl_command_queue mId;
};
	
}}
