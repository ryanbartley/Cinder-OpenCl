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
#include "cinder/Log.h"

namespace cinder { namespace cl {

static ContextRef sClContext = nullptr;
static bool sClContextInitialized = false;
	
Context::Context( const PlatformRef &platform, bool sharedGraphics, const ContextErrorCallback &errorCallback )
: mId( nullptr ), mIsGlShared( false )
{
	mIsGlShared = sharedGraphics && platform->isExtensionSupported( "cl_APPLE_gl_sharing" );
	mErrorCallback =  errorCallback ? errorCallback : &Context::contextErrorCallback;
	
	initialize(platform);
}

// This was made just to test something. The above should be used in all cases
Context::Context( bool sharedGl, const ContextErrorCallback &errorCallback )
: mId( nullptr ), mErrorCallback( errorCallback )
{
	auto platform = Platform::create( cl::Platform::getAvailablePlatforms()[0], true );
	
	mIsGlShared = sharedGl && platform->isExtensionSupported( "cl_APPLE_gl_sharing" );
	mErrorCallback =  errorCallback ? errorCallback : &Context::contextErrorCallback;

	initialize(platform);
	
}
	
cl_context_properties* Context::getDefaultSharedGraphicsContextProperties()
{
	static cl_context_properties contextProperties[] = { 0, 0, 0 };
#if defined (__APPLE__) || defined(MACOSX)
	contextProperties[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
	contextProperties[1] = (cl_context_properties)CGLGetShareGroup( ::CGLGetCurrentContext() ) ;
#elif defined(WIN32)
	CL_GL_CONTEXT_KHR , (cl_context_properties) wglGetCurrentContext() ,
    CL_WGL_HDC_KHR , (cl_context_properties) wglGetCurrentDC() ,
#endif
	//TODO: DirectX Implementation
	return contextProperties;
}
	
cl_context_properties* Context::getDefaultPlatformContextProperties( const PlatformRef &platform )
{
	static cl_context_properties contextProperties[] = {
		CL_CONTEXT_PLATFORM, 0, 0
	};
	contextProperties[1] = (cl_context_properties)platform->getId();
	return contextProperties;
}
	
void Context::initialize( const PlatformRef &platform )
{
	cl_int errNum = CL_SUCCESS;
	
	std::vector<cl_device_id> deviceIds;
	
	if ( mIsGlShared ) {
		mId = clCreateContext( getDefaultSharedGraphicsContextProperties(), 0, nullptr, mErrorCallback, this, &errNum );
		
		// TODO: This is messy but needs to happen because we don't know at
		// first which device Context will want to use for interoperability
		size_t ctsize;
		clGetContextInfo( mId, CL_CONTEXT_DEVICES, 0, nullptr, &ctsize );
		auto numDevices = ctsize / sizeof(cl_device_id);
		std::vector<cl_device_id> devices(numDevices);
		clGetContextInfo( mId, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * devices.size(), devices.data() , 0);
		
		for( const auto & deviceIdIt : devices ) {
			auto device = platform->getDeviceById( deviceIdIt );
			if( device != nullptr ) {
				std::cout << "We know about this device" << std::endl;
				mDeviceCommands.insert( make_pair( device, nullptr ) );
			}
			else
				CI_LOG_E("NOT A Device you know about");
		}
	}
	else {
		for( const auto & deviceIt : platform->getDevices() ) {
			deviceIds.push_back( deviceIt->getId() );
			mDeviceCommands.insert( make_pair( deviceIt, nullptr ) );
		}
		
		mId = clCreateContext( getDefaultPlatformContextProperties( platform ), deviceIds.size(), deviceIds.data(), mErrorCallback, this, &errNum );
	}
	
	// TODO:
	if( errNum != CL_SUCCESS ) {
		std::cerr << "Context was not created successfully " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
ContextRef Context::create( const PlatformRef &platform, bool sharedGl, const ContextErrorCallback &errorCallBack )
{
	sClContext = ContextRef( new Context( platform, sharedGl, errorCallBack ) );
	sClContextInitialized = true;
	return sClContext;
}
	
ContextRef Context::create( bool sharedGl, const ContextErrorCallback &errorCallback  )
{
	sClContext = ContextRef( new Context( sharedGl, errorCallback ) );
	sClContextInitialized = true;
	return sClContext;

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
	
std::vector<DeviceRef> Context::getAssociatedDevices()
{
	std::vector<DeviceRef> ret( mDeviceCommands.size() );
	auto beginIt = ret.begin();
	for( const auto &devComIt : mDeviceCommands ) {
		*beginIt++ = devComIt.first;
	}
	return ret;
}

ContextRef& Context::context()
{
	if ( ! sClContextInitialized ) {
		static ContextRef empty;
		return empty;
	}
	return sClContext;
}
	
}}