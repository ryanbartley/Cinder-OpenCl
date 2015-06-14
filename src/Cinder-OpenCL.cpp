//
//  Cinder-OpenCL.cpp
//  HelloWorld
//
//  Created by ryan bartley on 6/12/15.
//
//

#include "Cinder-OpenCL.h"
#include <OpenGL/OpenGL.h>
#include "cinder/gl/Context.h"
#include "cinder/Utilities.h"

namespace cinder {
	
std::ostream& operator<<( std::ostream &lhs, const cl::Platform &rhs )
{
	lhs << "CL_PLATFORM_NAME:   \t"	<< rhs.getInfo<CL_PLATFORM_NAME>() << "\n"
		<< "CL_PLATFORM_VENDOR: \t"	<< rhs.getInfo<CL_PLATFORM_VENDOR>() << "\n"
		<< "CL_PLATFORM_VERSION:\t" << rhs.getInfo<CL_PLATFORM_VERSION>() << "\n"
		<< "CL_PLATFORM_PROFILE:\t"	<< rhs.getInfo<CL_PLATFORM_PROFILE>() << "\n";
	auto extensions = std::string( rhs.getInfo<CL_PLATFORM_EXTENSIONS>() );
	auto extensionList = split( extensions, ' ' );
	int i = 0;
	lhs << "CL_PLATFORM_EXTENSIONS:\t" << extensionList[i++] << "\n";
	for( ; i < extensionList.size(); i++ ) {
		lhs << "                    \t" << extensionList[i] << "\n";
	}
	return lhs;
}
	
std::ostream& operator<<( std::ostream &lhs, const cl::Device &rhs )
{
	lhs << "CL_DEVICE_NAME:      \t" << rhs.getInfo<CL_DEVICE_NAME>() << "\n"
		<< "CL_DEVICE_VENDOR:    \t" << rhs.getInfo<CL_DEVICE_VENDOR>() << "\n"
		<< "CL_DRIVER_VERSION:   \t" << rhs.getInfo<CL_DRIVER_VERSION>() << "\n"
		<< "CL_DEVICE_PROFILE1:  \t" << rhs.getInfo<CL_DEVICE_PROFILE>() << "\n"
		<< "CL_DEVICE_VERSION:   \t" << rhs.getInfo<CL_DEVICE_VERSION>() << "\n"
		<< "CL_DEVICE_MAX_COMPUTE_UNITS:\t" << rhs.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << "\n";
	auto extensions = std::string( rhs.getInfo<CL_DEVICE_EXTENSIONS>() );
	auto extensionList = split( extensions, ' ' );
	int i = 0;
	lhs << "CL_DEVICE_EXTENSIONS:\t" << extensionList[i++] << "\n";
	for( ; i < extensionList.size(); i++ ) {
		lhs << "                    \t" << extensionList[i] << "\n";
	}
	return lhs;
}
	
cl_context_properties* getDefaultSharedGraphicsContextProperties()
{
	auto ctx = gl::context();
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
	
}