//
//  Platform.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/10/14.
//
//

#pragma once

#include <OpenCL/opencl.h>
#include "cinder/Exception.h"

namespace cl {
	
typedef std::shared_ptr<class Platform>						PlatformRef;
typedef std::shared_ptr<class Device>						DeviceRef;
typedef std::vector<DeviceRef>								DeviceList;
	
class Platform : public std::enable_shared_from_this<Platform>, public boost::noncopyable {
public:
	//! Returns a PlatformRef containing \a platform. Caches DeviceMap if \a cacheAllDevices is true. If \a platform is nullptr, it will create the Platform from the first available platform.
	static PlatformRef create( cl_platform_id platform, cl_device_type deviceType = CL_DEVICE_TYPE_GPU );
	//TODO: Fix this factory function.
	//! DON'T USE THIS FACTORY FUNCTION. Under repair. Returns a PlatformRef with \a platform as the platform, converting devices into DeviceRef's and filling the mDevices.
	static PlatformRef create( cl_platform_id platform, const std::vector<cl_device_id> &devices );
	
	~Platform();
	
	//! Returns cl_platform_id of the current contained platform.
	cl_platform_id					getId() const { return mId; }
	//! Returns a const vector reference of attached and activated devices.
	const DeviceList&				getDevices() const { return mDevices; }
	//! Returns a vector reference of attached and activated devices.
	DeviceList&						getDevices() { return mDevices; }
	//! Returns a vector of cl_device_id's of attached and activated devices.
	std::vector<cl_device_id>		getDeviceIds( cl_device_type type ) const;
	//! Returns a const DeviceRef for attached devices by type. The first one.
	const DeviceRef					getDeviceByType( cl_device_type type ) const;
	//! Returns a DeviceRef for attached devices by type. The first one.
	DeviceRef						getDeviceByType( cl_device_type type );
	DeviceRef						getDeviceById( cl_device_id deviceId );
	
	//! Returns whether an extension is supported on this platform.
	bool isExtensionSupported( const std::string &support_str );
	
	//! Returns a vector of available devices on this platform of this \a type and size of \a maxNumEntries. If \a maxNumEntries is 0 it will query and return all devices associated with this type. Normal options for type are CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_CPU.
	std::vector<cl_device_id> getAvailableDevices( cl_device_type type, cl_uint maxNumEntries = 0 );
		
		
	//! Returns a vector of cl_platform_id's from the current host.
	static std::vector<cl_platform_id>& getAvailablePlatforms();
	//! Prints out information on \a platformId, specifically, PROFILE, VERSION, VENDOR, EXTENSIONS. Uses getPlatformInfo to print out the most needed info on a platform
	static void displayPlatformInfo( cl_platform_id platformId );
	//! Prints out information on \a platform, specifically, PROFILE, VERSION, VENDOR, EXTENSIONS. Uses getPlatformInfo to print out the most needed info on a platform
	static void displayPlatformInfo( const PlatformRef &platform );
	//! Returns string of the value of the provided \a platformId and \a paramName enumerator.
	static std::string getPlatformParam( cl_platform_id platformId, cl_platform_info paramName );
	
private:
	Platform( cl_platform_id platform );
	Platform( cl_platform_id platform, const std::vector<DeviceRef>& devices );
		
	void cacheDevices();
	
	cl_platform_id	mId;
	DeviceList		mDevices;
	
	friend std::ostream& operator<<( std::ostream &lhs, const Platform &rhs );
};
	
std::ostream& operator<<( std::ostream &lhs, const PlatformRef &rhs );
std::ostream& operator<<( std::ostream &lhs, const cl_platform_id &platformId );
	
class OpenCLException : public ci::Exception {
public:
	OpenCLException() : Exception() {}
	OpenCLException( const std::string &message ) : Exception( message ) {}
};
	
class PlatformException : public OpenCLException {
public:
	PlatformException() : OpenCLException() {}
	PlatformException( const std::string &message ) : OpenCLException( message ) {}
};

class PlatformDeviceExc : public PlatformException {
public:
	PlatformDeviceExc() : PlatformException() {}
	PlatformDeviceExc( const std::string &message ) : PlatformException( message ) {}
};
	
} // namespace cl
