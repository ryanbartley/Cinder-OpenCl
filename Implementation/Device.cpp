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

Device::Device( const PlatformRef &platform, cl_device_type type )
{
	
}
	
Device::Device( cl_device_id device )
: mId( device )
{
	cl_int errNum;
	errNum = clGetDeviceInfo( device, CL_DEVICE_TYPE, sizeof(cl_device_type), &mType, nullptr);
	if( errNum != CL_SUCCESS ) {
		std::cout << "Problem finding the Device Type " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
DeviceRef Device::create( cl_device_id device )
{
	return DeviceRef( new Device( device ) );
}

Device::~Device()
{
}
	


std::vector<cl_device_id> Device::getAvailableDevices( cl_platform_id platform, cl_device_type type )
{
	cl_int errNum;
	cl_uint numDevices;
	
	errNum = clGetDeviceIDs( platform, type, 0, NULL, &numDevices);
	
	if (numDevices < 1)
	{
		std::cout << "No GPU device found for platform " << platform << std::endl;
		exit(1);
	}
	
	std::vector<cl_device_id> devices( numDevices );
	
	errNum = clGetDeviceIDs( platform, type, numDevices, devices.data(), NULL);
	
	return devices;
}

std::vector<cl_device_id> Device::getAvailableDevices( const PlatformRef &platform, cl_device_type type )
{
	return getAvailableDevices(platform->getId(), type);
}
	
}}