//
//  Context.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#include "Context.h"
#include <OpenGL/OpenGL.h>
#include "Platform.h"
#include "Device.h"

namespace cinder { namespace cl {

static Context* sClContext = nullptr;
static bool sClContextInitialized = false;
	
Context::Context(const PlatformRef &platform, bool sharedGraphics, const ContextErrorCallback &errorCallback )
: mIsGlShared( false ), mPlatform( platform )
{
	cl_int errNum;
	// TODO: Need to create DirectX implementation
	if( ! sharedGraphics ) {
		cl_context_properties contextProperties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)platform->getId(), 0
		};
		
		auto devices = platform->getDevices();
		
		std::vector<cl_device_id> deviceIds;
		
		for( auto deviceIdIt = devices.begin(); deviceIdIt != devices.end(); ++deviceIdIt ) {
			deviceIds.push_back((*deviceIdIt)->getId());
			std::cout << "I'm adding things " << (*deviceIdIt)->getId() << std::endl;
		}
		
		mId = clCreateContext( contextProperties, deviceIds.size(), deviceIds.data(), &Context::contextErrorCallback, this, &errNum );
		
		if( errNum != CL_SUCCESS ) {
			std::cerr << "Context was not created successfully " << errNum << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else {
		// TODO: DirectX Implementation through ifdefs
		CGLContextObj kCGLContext = ::CGLGetCurrentContext();
		CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup( kCGLContext );
		cl_context_properties contextProperties[] = {
			CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
			(cl_context_properties)kCGLShareGroup, 0
		};
		
		mId = clCreateContext( contextProperties, 0, 0, nullptr, nullptr, &errNum );
		
		if( errNum != CL_SUCCESS ) {
			std::cerr << "Context was not created successfully " << errNum << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	
	sClContext = this;
	sClContextInitialized = true;
}

// This was made just to test something. The above should be used in all cases
Context::Context()
{
	auto platforms = cl::Platform::getAvailablePlatforms();
	auto devices = cl::Device::getAvailableDevices( platforms[0], CL_DEVICE_TYPE_ALL );
	
	cl_int errNum;
	
	cl_context_properties contextProperties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[0], 0
	};
	
	mId = clCreateContext( contextProperties, devices.size(), devices.data(), NULL, NULL, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		std::cerr << "Context was not created successfully " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
	
}
	
ContextRef Context::create( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallBack )
{
	return ContextRef( new Context( platform, sharedGl, errorCallBack ) );
}
	
ContextRef Context::create()
{
	return ContextRef( new Context() );
}

Context::~Context()
{
	clReleaseContext(mId);
	sClContext = nullptr;
	sClContextInitialized = false;
}
	
void Context::contextErrorCallback( const char *errInfo, const void *privateInfo, size_t cb, void *userData )
{
	std::cout << "Error occured during context use: " << errInfo << std::endl;
	exit(EXIT_FAILURE);
}

Context* Context::context()
{
	if ( ! sClContextInitialized ) {
		return nullptr;
	}
	return sClContext;
}
	
}}