//
//  Device.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/14/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "DeviceInfo.hpp"

namespace cinder { namespace cl {

typedef std::shared_ptr<class Platform> PlatformRef;
typedef std::shared_ptr<class Device> DeviceRef;

class Device {
public:
	static DeviceRef create( cl_device_id device );
	static DeviceRef create( const PlatformRef &platform, cl_device_type type );
	~Device();
	
	cl_device_id getId() const { return mId; }
	cl_device_type getType() const { return mType; }
	
	
	template<typename T>
	T getInfo( cl_device_info name );
	
	static std::vector<cl_device_id> getAvailableDevices( const PlatformRef &platform, cl_device_type type );
	static std::vector<cl_device_id> getAvailableDevices( cl_platform_id platform, cl_device_type type );
	
	template <typename T>
	static void displayDeviceInfo( cl_device_id id, cl_device_info name, std::string str );
	static bool isExtensionSupported( const char *support_str, const char *ext_string, size_t ext_buffer_size );
	
private:
	Device( cl_device_id device );
	Device( const PlatformRef &platform, cl_device_type type );
	
	cl_device_id	mId;
	cl_device_type	mType;
};
	
template<typename T>
T Device::getInfo( cl_device_info info )
{
	
}
	
template<typename T>
void Device::displayDeviceInfo(cl_device_id id, cl_device_info name, std::string str)
{
	InfoDevice<T>::display( id, name, str );
}
	
}}