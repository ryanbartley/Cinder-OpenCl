//
//  Device.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/14/14.
//
//

#include "Device.h"
#include "Platform.h"


namespace cinder { namespace cl {
	
Device::Device( const PlatformRef &platform, cl_device_id device )
: mId( device ), mParentPlatform( platform )
{
	cl_int errNum;
	errNum = clGetDeviceInfo( device, CL_DEVICE_TYPE, sizeof(cl_device_type), &mType, nullptr);
	
	// TODO: Add errnum check and Throw DeviceExc
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "Problem finding the Device Type " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
	
	// Figure out what to do with this
	// It needs to be like a waterfall for this option
#if defined (__APPLE__) || defined(MACOSX)
	static const char* CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
	static const char* CL_GL_EVENT_EXT = "cl_khr_gl_event";
#else
	static const char* CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
	static const char* CL_GL_EVENT_EXT = "cl_khr_gl_event";
#endif
	isExtensionSupported( CL_GL_SHARING_EXT );
	isExtensionSupported( CL_GL_EVENT_EXT );
}
	
Device::~Device()
{
	clReleaseDevice(mId);
}
	
std::string Device::getSupportedExtensions( cl_device_id device )
{
	std::string extensions;
	if( extensions.empty() ) {
		size_t ext_size = 1024;
		extensions.resize( 1024 );
		cl_int errNum = clGetDeviceInfo( device, CL_DEVICE_EXTENSIONS, ext_size, &extensions[0], &ext_size);
		// TODO: Add errnum check and Throw DeviceExc
		if( errNum != CL_SUCCESS ) {
			std::string error( "Error: getSupportedExtensions: errNum = " );
			error += Platform::getClErrorString( errNum );
			std::cout << "Error: getSupportedExtensions" << std::endl;
		}
	}
	return extensions;
}
	
void Device::displayDeviceSupportedExtensions( const DeviceRef &device )
{
	displayDeviceSupportedExtensions( device->getId() );
}
	
void Device::displayDeviceSupportedExtensions( cl_device_id deviceId )
{
	InfoDevice<ArrayType<char>>::display( deviceId, CL_DEVICE_EXTENSIONS, "Current Device extensions" );
}
	
DeviceRef Device::create( const PlatformRef &platform, cl_device_id device )
{
	return DeviceRef( new Device( platform, device ) );
}

bool Device::isExtensionSupported( const std::string &support_str )
{
	static std::map<std::string, bool> extensionSupport;
	static std::string ext_string;
	if ( ext_string.empty() ) {
		ext_string = getSupportedExtensions( mId );
	}
	
	auto found = extensionSupport.find( support_str );
	
	if( found == extensionSupport.end() ) {
		size_t pos = ext_string.find( support_str );
		bool supported = ( pos != std::string::npos );
		auto newExtension = extensionSupport.insert( std::pair<std::string, bool>( support_str, supported ) );
		return newExtension.first->second;
	}
	else {
		return found->second;
	}}
	
const char* Device::getDeviceTypeString( cl_device_type type )
{
	switch (type) {
		case CL_DEVICE_TYPE_ACCELERATOR:
			return "CL_DEVICE_TYPE_ACCELERATOR";
			break;
		case CL_DEVICE_TYPE_ALL:
			return "CL_DEVICE_TYPE_ALL";
			break;
		case CL_DEVICE_TYPE_CPU:
			return "CL_DEVICE_TYPE_CPU";
			break;
		case CL_DEVICE_TYPE_CUSTOM:
			return "CL_DEVICE_TYPE_CUSTOM";
			break;
		case CL_DEVICE_TYPE_DEFAULT:
			return "CL_DEVICE_TYPE_DEFAULT";
			break;
		case CL_DEVICE_TYPE_GPU:
			return "CL_DEVICE_TYPE_GPU";
			break;
		default:
			return "UNKNOWN DEVICE TYPE";
			break;
	}
}
	
}}