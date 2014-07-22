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

class Device : public std::enable_shared_from_this<Device>, public boost::noncopyable {
public:
	static DeviceRef create( const PlatformRef &platform, cl_device_id device );
	static DeviceRef create( const PlatformRef &platform, cl_device_type type );
	~Device();
	
	cl_device_id		getId() const { return mId; }
	cl_device_type		getType() const { return mType; }
	const PlatformRef&	getPlatform() const { return mParentPlatform; }
	PlatformRef&		getPlatform() { return mParentPlatform; }
	
	bool isExtensionSupported( const std::string &support_str );
	
	template<typename T>
	T getInfo( cl_device_info name );
	
	template <typename T>
	static void displayDeviceInfo( cl_device_id deviceId, cl_device_info name, std::string str );
	static const char* getDeviceTypeString( cl_device_type type );
	
	static void displayDeviceSupportedExtensions( const DeviceRef &device );
	static void displayDeviceSupportedExtensions( cl_device_id deviceId );
	static std::string getSupportedExtensions( const DeviceRef &device );
	static std::string getSupportedExtensions( cl_device_id device );
	
private:
	Device( const PlatformRef &platform, cl_device_id device );
	Device( const PlatformRef &platform, cl_device_type type );
	
	PlatformRef		mParentPlatform;
	cl_device_id	mId;
	cl_device_type	mType;
	
	friend class Platform;
};
	
template<typename T>
T Device::getInfo( cl_device_info info )
{
	
}
	
template<typename T>
void Device::displayDeviceInfo(cl_device_id deviceId, cl_device_info name, std::string str)
{
	InfoDevice<T>::display( deviceId, name, str );
}
	
}}