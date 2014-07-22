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

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class Context> ContextRef;
typedef std::shared_ptr<class Platform> PlatformRef;
typedef std::shared_ptr<class Device> DeviceRef;
typedef void(*ContextErrorCallback)(const char * errInfo, const void * privateInfo, size_t cb, void * userData);
	
class Context : public boost::noncopyable, public std::enable_shared_from_this<Context> {
public:
	static ContextRef create( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallBack = nullptr );
	static ContextRef create( bool sharedGl, const ContextErrorCallback &errorCallback = nullptr );
	
	~Context();
	
	static Context* context();
	
	cl_context				getId() const { return mId; }
	bool					isGlShared() const { return mIsGlShared; }
	std::vector<DeviceRef>& getAssociatedDevices() { return mDevices; }
	
private:
	Context( bool sharedGl, const ContextErrorCallback &errorCallback );
	Context( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallback );
	static void CL_CALLBACK contextErrorCallback( const char * errInfo, const void * privateInfo, size_t cb, void * userData );
	
	void initialize( const PlatformRef &platform );
	static cl_context_properties* getDefaultSharedGraphicsContextProperties();
	static cl_context_properties* getDefaultPlatformContextProperties( const PlatformRef &platform );
	
	cl_context				mId;
	bool					mIsGlShared;
	std::vector<DeviceRef>	mDevices;
	ContextErrorCallback	mErrorCallback;
	
};
	
}}
