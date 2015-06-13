//
//  Context.h
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#pragma once

#include <OpenCL/opencl.h>
#include "cinder/gl/Context.h"

namespace cl {
	
using ContextRef			= std::shared_ptr<class Context>;
using PlatformRef			= std::shared_ptr<class Platform>;
using DeviceRef				= std::shared_ptr<class Device>;
using CommandQueueRef		= std::shared_ptr<class CommandQueue>;
using DeviceCommandMap		= std::map<DeviceRef, CommandQueueRef>;
using ContextErrorCallback	= void(*)(const char * errInfo,
									  const void * privateInfo,
									  size_t cb, void * userData);
	
class Context : public std::enable_shared_from_this<Context> {
public:
	static ContextRef create( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallBack = nullptr );
	static ContextRef create( bool sharedGl, const ContextErrorCallback &errorCallback = nullptr );
	
	~Context();
	
	static ContextRef& context();
	
	cl_context				getId() const { return mId; }
	bool					isGlShared() const { return mIsGlShared; }
	std::vector<DeviceRef>	getAssociatedDevices();
	
	// NON COPYABLE
	Context( const Context &other ) = delete;
	Context& operator=( const Context &other ) = delete;
	Context( Context&& other ) = delete;
	Context& operator=( Context&& other ) = delete;
	
private:
	Context( bool sharedGl, const ContextErrorCallback &errorCallback );
	Context( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallback );
	
	static void CL_CALLBACK contextErrorCallback( const char * errInfo, const void * privateInfo, size_t cb, void * userData );
	
	void initialize( const PlatformRef &platform );
	static cl_context_properties* getDefaultSharedGraphicsContextProperties();
	static cl_context_properties* getDefaultPlatformContextProperties( const PlatformRef &platform );
	
	cl_context				mId;
	bool					mIsGlShared;
	DeviceCommandMap		mDeviceCommands;
	ContextErrorCallback	mErrorCallback;
	
};
	
} // namespace cl
