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
#include "BufferObj.h"
#include "Event.h"
#include "Platform.h"
#include "Program.h"


namespace cinder { namespace cl {

CommandQueue::CommandQueue( const DeviceRef &device, cl_command_queue_properties properties  )
: mId( nullptr ), mProperties( properties ), mDevice( device ), mContext( Context::context() )
{
	cl_int errNum;
	
	mId = clCreateCommandQueue( mContext->getId(), mDevice->getId(), properties, &errNum );

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
	
void CommandQueue::acquireGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents, Event *returnEvent )
{
	if ( ! mContext->isGlShared() ) {
		std::cout << "ERROR: You're Context is not sharing" << std::endl;
		return;
	}
		
	
	cl_int errNum = CL_SUCCESS;
	std::vector<cl_mem> memObjs(acquireObjs.size());
	
	std::transform( acquireObjs.begin(), acquireObjs.end(), memObjs.begin(),
					[&](const MemoryObjRef& memObj){
						return memObj->getId();
					});
	
	auto events = waitEvents.getEventIdList();
	
	errNum = clEnqueueAcquireGLObjects( mId, memObjs.size(), memObjs.data(), events.size(), events.data(), returnEvent ? (*returnEvent) : nullptr );
	if ( errNum) {
		std::cout << "ERROR: there's been an error, " << Platform::getErrorString(errNum) << std::endl;
	}
}

void CommandQueue::releaseGlObjects( std::vector<MemoryObjRef>& acquireObjs, const EventList &waitEvents, Event *returnEvent )
{
	if ( ! mContext->isGlShared() ) {
		std::cout << "ERROR: You're Context is not sharing" << std::endl;
		return;
	}
	
	
	cl_int errNum = CL_SUCCESS;
	std::vector<cl_mem> memObjs(acquireObjs.size());
	
	std::transform( acquireObjs.begin(), acquireObjs.end(), memObjs.begin(),
				   [&](const MemoryObjRef& memObj){
					   return memObj->getId();
				   });
	
	auto events = waitEvents.getEventIdList();
	
	errNum = clEnqueueAcquireGLObjects( mId, memObjs.size(), memObjs.data(), events.size(), events.data(), returnEvent ? (*returnEvent) : nullptr );
	if ( errNum) {
		std::cout << "ERROR: there's been an error, " << Platform::getErrorString(errNum) << std::endl;
	}
}

void CommandQueue::write( const BufferObjRef &buffer, cl_bool blockWrite, size_t offset, size_t size, void *data, const EventList &eventWaitList, Event *returnEvent )
{
	cl_int errNum;
	
	auto eventIdList = eventWaitList.getEventIdList();
	
	errNum = clEnqueueWriteBuffer( mId, buffer->getId(), blockWrite, offset, size, data, eventIdList.size(), eventIdList.data(), returnEvent ? (*returnEvent) : nullptr  );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
void CommandQueue::read( const BufferObjRef &buffer, cl_bool blockRead, size_t offset, size_t size, void *data, const EventList &eventWaitList, Event *returnEvent )
{
	cl_int errNum;
	
	auto eventIdList = eventWaitList.getEventIdList();
	
	errNum = clEnqueueReadBuffer( mId, buffer->getId(), blockRead, offset, size, data, eventIdList.size(), eventIdList.data(), returnEvent ? (*returnEvent) : nullptr  );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
void CommandQueue::NDRangeKernel(	const KernelRef &kernel, cl_uint work_dim, size_t *global_work_offset, size_t *global_work_size, size_t *local_work_size, const EventList &waitList, Event *returnEvent )
{
	cl_int errNum;
	
	auto eventIdList = waitList.getEventIdList();
	std::cout << eventIdList.size() << std::endl;
	
	errNum = clEnqueueNDRangeKernel( mId,
									kernel->getId(),
									work_dim,
									global_work_offset,
									global_work_size,
									local_work_size,
									eventIdList.size(),
									eventIdList.size() ? eventIdList.data() : nullptr,
									returnEvent ? (*returnEvent) : nullptr );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: Write to buffer Failed " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
}}