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
	~Device();
	
	//! Returns the cl_device_id contained in this Device
	cl_device_id		getId() const { return mId; }
	//! Returns the cl_device_type associated with this Device
	cl_device_type		getType() const { return mType; }
	//! Returns a const reference to the parent platform
	const PlatformRef&	getPlatform() const { return mParentPlatform; }
	//! Returns a reference to the parent platform
	PlatformRef&		getPlatform() { return mParentPlatform; }
	//! Returns whether this Device supports cl/gl sharing
	bool isGlSupported() { return mIsGlSupported; }
	//! Returns whether this Device supports this extension.
	bool isExtensionSupported( const std::string &support_str );
	
	template <typename T>
	static void displayDeviceInfo( cl_device_id deviceId, cl_device_info name, std::string str );
	static const char* getDeviceTypeString( cl_device_type type );
	
	static void displayDeviceSupportedExtensions( const DeviceRef &device );
	static void displayDeviceSupportedExtensions( cl_device_id deviceId );
	static std::string getSupportedExtensions( const DeviceRef &device );
	static std::string getSupportedExtensions( cl_device_id device );
	
	template<typename T>
	static void getDeviceInfo( cl_device_id deviceId, cl_device_info param_name, void *returnData );
	
private:
	Device( const PlatformRef &platform, cl_device_id device );
	
	static DeviceRef create( const PlatformRef &platform, cl_device_id device );
	
	cl_device_id	mId;
	cl_device_type	mType;
	PlatformRef		mParentPlatform;
	std::map<std::string, bool> mExtensionSupport;
	std::string					mExtensionString;
	bool			mIsGlSupported;
	
	friend class Platform;
};
	
template<typename T>
void Device::getDeviceInfo( cl_device_id deviceId, cl_device_info param_name, void *returnData )
{
	cl_int errNum;
	
	errNum = clGetDeviceInfo( deviceId, param_name, sizeof(T), returnData, NULL );
	if (errNum != CL_SUCCESS) {
		std::cerr << "Failed to find OpenCL device info." << std::endl;//<< Platform::getClErrorString( errNum ) << std::endl;
		return;
	}
}
	
template<>
void Device::getDeviceInfo<char>( cl_device_id deviceId, cl_device_info param_name, void *returnData )
{
	cl_int errNum;
	std::size_t paramValueSize;
	
	errNum = clGetDeviceInfo( deviceId, param_name, 0, NULL, &paramValueSize );
	if (errNum != CL_SUCCESS) {
		std::cerr << "Failed to find OpenCL device info." << std::endl;
		return;
	}
	
	const char * info = new char[paramValueSize];
	
	errNum = clGetDeviceInfo( deviceId, param_name, paramValueSize, &info, NULL );
	if (errNum != CL_SUCCESS) {
		std::cerr << "Failed to find OpenCL device info." << std::endl;
		return;
	}
	
	if( std::string * returnString = static_cast<std::string*>(returnData) ) {
		returnString->clear();
		returnString->insert( 0, info );
	}
}
	
template<typename T>
void Device::displayDeviceInfo(cl_device_id deviceId, cl_device_info name, std::string str)
{
	InfoDevice<T>::display( deviceId, name, str );
}
	
}}