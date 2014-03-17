//
//  Platform.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/10/14.
//
//

#include "Platform.h"
#include "Device.h"

namespace cinder { namespace cl {
	
Platform::Platform( cl_platform_id platform, const std::vector<DeviceRef> &devices )
: mId( platform ), mDevices( devices )
{
	
}
	
PlatformRef Platform::create( cl_platform_id platform, const std::vector<cl_device_id> &deviceIds )
{
	std::vector<DeviceRef> devices;
	for( auto deviceIt = deviceIds.begin(); deviceIt != deviceIds.end(); ++deviceIt ) {
		devices.push_back( Device::create( *deviceIt ) );
	}
	return PlatformRef( new Platform( platform, devices ) );
}

PlatformRef Platform::create( cl_platform_id platform, const std::vector<DeviceRef> &devices )
{
	return PlatformRef( new Platform( platform, devices ) );
}

std::vector<cl_platform_id> Platform::getAvailablePlatforms()
{
	cl_int errNum;
	cl_uint numPlatforms;
	
	errNum = clGetPlatformIDs( 0, NULL, &numPlatforms );
	
	if( errNum != CL_SUCCESS ) {
		throw "could not get Platforms";
	}
	
	std::vector<cl_platform_id> platforms(numPlatforms);
	
	errNum = clGetPlatformIDs( numPlatforms, platforms.data(), NULL );
	
	if( errNum != CL_SUCCESS ) {
		throw "platforms could not be queried";
	}
	
	return platforms;
}

Platform::~Platform()
{
}

void Platform::displayPlatformInfo( cl_platform_id platformId )
{
	getPlatformInfo( platformId, CL_PLATFORM_PROFILE, "CL_PLATFORM_PROFILE" );
	getPlatformInfo( platformId, CL_PLATFORM_VERSION, "CL_PLATFORM_VERSION" );
	getPlatformInfo( platformId, CL_PLATFORM_VENDOR, "CL_PLATFORM_VENDOR" );
	getPlatformInfo( platformId, CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS" );
}

void Platform::getPlatformInfo( cl_platform_id id, cl_platform_info name, const std::string &str )
{
	cl_int errNum;
	
	std::size_t paramValueSize;
	
	errNum = clGetPlatformInfo( id, name, 0, NULL, &paramValueSize );
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Failed to find OpenCL platform " << str << "." << std::endl;
		return;
	}
	char * info = (char *)alloca(sizeof(char) * paramValueSize);
	
	errNum = clGetPlatformInfo( id, name, paramValueSize, info, NULL );
	if (errNum != CL_SUCCESS)
	{
		std::cerr << "Failed to find OpenCL platform " << str << "." << std::endl;
		return;
	}
	
	std::cout << "\t" << str << ":\t" << info << std::endl;
}

}}