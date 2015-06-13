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
#include "ConstantConversion.h"

namespace cl {

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
	
void CommandQueue::barrierWithWaitlist( const EventList &list, Event *event )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	errNum = clEnqueueBarrierWithWaitList( mId, waitList.size(), waitList.data(), event ? (*event) : nullptr );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void CommandQueue::marker( Event *event )
{
	cl_int errNum;
	errNum = clEnqueueMarker( mId, *event );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void CommandQueue::waitForEvents( const EventList &list )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	errNum = clEnqueueWaitForEvents( mId, waitList.size(), waitList.data() );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
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
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
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
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}

void CommandQueue::write( const BufferObjRef &buffer, cl_bool blockWrite, size_t offset, size_t size, void *data, const EventList &eventWaitList, Event *returnEvent )
{
	cl_int errNum;
	
	auto eventIdList = eventWaitList.getEventIdList();
	
	errNum = clEnqueueWriteBuffer( mId, buffer->getId(), blockWrite, offset, size, data, eventIdList.size(), eventIdList.data(), returnEvent ? (*returnEvent) : nullptr  );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void CommandQueue::read( const BufferObjRef &buffer, cl_bool blockRead, size_t offset, size_t size, void *data, const EventList &eventWaitList, Event *returnEvent )
{
	cl_int errNum;
	
	auto eventIdList = eventWaitList.getEventIdList();
	
	errNum = clEnqueueReadBuffer( mId, buffer->getId(), blockRead, offset, size, data, eventIdList.size(), eventIdList.data(), returnEvent ? (*returnEvent) : nullptr  );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void CommandQueue::fill( const BufferObjRef &buffer, void* pattern, size_t patternSize, size_t offset, size_t size, const EventList &list, Event *returnEvent )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	errNum = clEnqueueFillBuffer( mId, buffer->getId(), pattern, patternSize, offset, size, waitList.size(), waitList.data(), returnEvent ? (*returnEvent) : nullptr );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void CommandQueue::copy( const BufferObjRef &srcBuffer, const BufferObjRef &dstBuffer, size_t srcOffset, size_t dstOffset, size_t size, const EventList &list, Event *returnEvent )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	errNum = clEnqueueCopyBuffer ( mId, srcBuffer->getId(), dstBuffer->getId(), srcOffset, dstOffset, size, waitList.size(), waitList.data(), returnEvent ? (*returnEvent) : nullptr );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
void* CommandQueue::map( const BufferObjRef &buffer, cl_bool blockMap, cl_map_flags mapFlags, size_t offset, size_t size, const EventList &list, Event *returnEvent )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	auto amountToWrite = size == -1 ? (buffer->getSize() - offset) : size;
	
	auto ret = clEnqueueMapBuffer ( mId, buffer->getId(), blockMap, mapFlags, offset, amountToWrite, waitList.size(), waitList.data(), returnEvent ? (*returnEvent) : nullptr, &errNum );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
	return ret;
}
	
void CommandQueue::unmap( const MemoryObjRef &memObj, void* mappedPointer, const EventList &list, Event *returnEvent )
{
	cl_int errNum;
	
	auto waitList = list.getEventIdList();
	
	errNum = clEnqueueUnmapMemObject( mId, memObj->getId(), mappedPointer, waitList.size(), waitList.data(), returnEvent ? (*returnEvent) : nullptr );
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
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
	
	if ( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << getErrorString( errNum ) << std::endl;
	}
}
	
}