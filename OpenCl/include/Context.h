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
typedef std::function<void(const char * errInfo, const void * privateInfo, size_t cb, void * userData)> ContextErrorCallback;
	
class Context : public boost::noncopyable, public std::enable_shared_from_this<Context> {
public:
	static ContextRef create( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallBack = nullptr );
	static ContextRef create();
	
	~Context();
	
	static Context* context();
	
	const cl_context	getId() { return mId; }
	bool				isGlShared() { return mIsGlShared; }
	
private:
	Context();
	Context( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallback );
	static void CL_CALLBACK contextErrorCallback( const char * errInfo, const void * privateInfo, size_t cb, void * userData );
	
	PlatformRef mPlatform;
	cl_context	mId;
	bool		mIsGlShared;
	
	
};
	
}}
