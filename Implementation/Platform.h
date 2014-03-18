//
//  Platform.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/10/14.
//
//

#pragma once

#include <OpenCL/opencl.h>

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class Platform> PlatformRef;
typedef std::shared_ptr<class Device> DeviceRef;
	
class Platform {
public:
	
	static PlatformRef create( cl_platform_id platform, const std::vector<cl_device_id> &devices );
	static PlatformRef create( cl_platform_id platform, const std::vector<DeviceRef> &device );
	
	~Platform();
	
	cl_platform_id					getId() { return mId; }
	const std::vector<DeviceRef>&	getDevices() { return mDevices; }
	std::vector<cl_device_id>		getDeviceIds();
	
	static void displayPlatformInfo( cl_platform_id platformId );
	
	static void getPlatformInfo( cl_platform_id pId, cl_platform_info pName, const std::string &pNameStr );
	static std::vector<cl_platform_id> getAvailablePlatforms();
	
private:
	Platform( cl_platform_id platform, const std::vector<DeviceRef>& devices );
	
	cl_platform_id				mId;
	std::vector<DeviceRef>		mDevices;
	
	friend std::ostream& operator<<( std::ostream &lhs, const Platform &rhs );
};
	
std::ostream& operator<<( std::ostream &lhs, const Platform &rhs )
{
	return lhs;
}
	
}}
