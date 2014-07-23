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

static ContextRef sClContext = nullptr;
static bool sClContextInitialized = false;
	
Context::Context( const PlatformRef &platform, bool sharedGraphics, const ContextErrorCallback &errorCallback )
: mId( nullptr ), mIsGlShared( false ), mDevices( platform->getDevices() )
{
	mIsGlShared = sharedGraphics && platform->isExtensionSupported( "cl_APPLE_gl_sharing" );
	mErrorCallback =  errorCallback ? errorCallback : &Context::contextErrorCallback;
	
	initialize(platform);
}

// This was made just to test something. The above should be used in all cases
Context::Context( bool sharedGl, const ContextErrorCallback &errorCallback )
: mId( nullptr ), mErrorCallback( errorCallback )
{
	auto platform = Platform::create( cl::Platform::getAvailablePlatforms()[0] );
	auto devices = platform->getAvailableDevices( CL_DEVICE_TYPE_GPU );
	
	for( auto deviceIdIt = devices.begin(); deviceIdIt != devices.end(); ++deviceIdIt ) {
		mDevices.push_back( Device::create( platform, (*deviceIdIt) ) );
		std::cout << "I'm adding things " << mDevices.back()->getId() << std::endl;
	}
	
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
	//TODO: DirectX Implementation
	CL_GL_CONTEXT_KHR , (cl_context_properties) wglGetCurrentContext() ,
    CL_WGL_HDC_KHR , (cl_context_properties) wglGetCurrentDC() ,
#endif
	return contextProperties;
}
	
cl_context_properties* Context::getDefaultPlatformContextProperties( const PlatformRef &platform )
{
	static cl_context_properties contextProperties[] = {
		CL_CONTEXT_PLATFORM, 0, 0
	};
	contextProperties[2] = (cl_context_properties)platform->getId();
	return contextProperties;
}
	
void Context::initialize( const PlatformRef &platform )
{
	cl_int errNum = CL_SUCCESS;
	
	std::vector<cl_device_id> deviceIds;
	
	for( auto deviceIdIt = mDevices.begin(); deviceIdIt != mDevices.end(); ++deviceIdIt ) {
		deviceIds.push_back((*deviceIdIt)->getId());
		std::cout << "I'm adding things " << (*deviceIdIt)->getId() << std::endl;
	}
	
	if ( mIsGlShared ) {
		
		auto id = mDevices.front()->getId();
		mId = clCreateContext( getDefaultSharedGraphicsContextProperties(), 1, &id, mErrorCallback, this, &errNum );
	}
	else {
		mId = clCreateContext( getDefaultPlatformContextProperties( mDevices[0]->getPlatform() ), deviceIds.size(), deviceIds.data(), mErrorCallback, this, &errNum );
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

ContextRef& Context::context()
{
	if ( ! sClContextInitialized ) {
		static ContextRef empty;
		return empty;
	}
	return sClContext;
}
	
}}