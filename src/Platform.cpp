//
//  Platform.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/10/14.
//
//

#include "Platform.h"
#include "Device.h"
#include "ConstantConversion.h"

using namespace std;

namespace cl {

Platform::Platform( cl_platform_id platform )
: mId( platform ? platform : getAvailablePlatforms()[0] )
{
}
	
	
Platform::Platform( cl_platform_id platform, const std::vector<DeviceRef> &devices )
: mId( platform )
{
	
}
	
Platform::~Platform()
{
}
	
PlatformRef Platform::create( cl_platform_id platform, cl_device_type deviceType )
{
	auto ret = PlatformRef( new Platform( platform ) );
	auto devices = ret->getAvailableDevices( deviceType );
	DeviceList deviceList;
	std::transform( devices.begin(), devices.end(), std::back_inserter( deviceList ),
	[ret]( const cl_device_id & device ) {
		return Device::create( ret, device );
	});
	ret->mDevices = deviceList;
	return ret;
}
	
std::vector<cl_device_id> Platform::getDeviceIds( cl_device_type deviceType ) const
{
	auto devices = getDevices();
	std::vector<cl_device_id> deviceIds;
	for( auto deviceIdIts = devices.begin(); deviceIdIts != devices.end(); ++deviceIdIts ) {
		if( deviceType == CL_DEVICE_TYPE_ALL || (*deviceIdIts)->getType() == deviceType )
			deviceIds.push_back( (*deviceIdIts)->getId() );
	}
	return deviceIds;
}

DeviceRef Platform::getDeviceById( cl_device_id deviceId )
{
	auto devices = getDevices();
	for ( auto deviceIts = devices.begin(); deviceIts != devices.end(); ++deviceIts ) {
		if ( deviceId == (*deviceIts)->getId() ) {
			return (*deviceIts);
		}
	}
	return DeviceRef();
}

const DeviceRef Platform::getDeviceByType( cl_device_type type ) const
{
	for( auto deviceIts = mDevices.begin(); deviceIts != mDevices.end(); ++deviceIts ) {
		if( (*deviceIts)->getType() == type ) {
			return (*deviceIts);
		}
	}
	return DeviceRef();
}

DeviceRef Platform::getDeviceByType( cl_device_type type )
{
	for( auto deviceIts = mDevices.begin(); deviceIts != mDevices.end(); ++deviceIts ) {
		if( (*deviceIts)->getType() == type ) {
			return (*deviceIts);
		}
	}
	return DeviceRef();
}
	
bool Platform::isExtensionSupported( const std::string &support_str )
{
	static std::map<std::string, bool> extensionSupport;
	static std::string ext_string;
	if ( ext_string.empty() ) {
		ext_string = getPlatformParam( mId, CL_PLATFORM_EXTENSIONS );
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
	}
}
	
void Platform::cacheDevices()
{
	auto devices = getAvailableDevices( CL_DEVICE_TYPE_ALL );
	mDevices.resize( devices.size() );
	std::transform( devices.begin(), devices.end(), mDevices.begin(),
				   [&]( cl_device_id deviceId ) {
					   return Device::create( shared_from_this(), deviceId );
				   });
}
	
std::vector<cl_device_id> Platform::getAvailableDevices( cl_device_type type, cl_uint maxNumEntries )
{
	cl_int errNum;
	cl_uint numDevices = maxNumEntries;
	
	if( numDevices == 0 ) {
		errNum = clGetDeviceIDs( mId, type, 0, NULL, &numDevices);
		
		// TODO: Add errNum check here
		if ( numDevices < 1 ) {
			std::string excString;
			excString += "No ";
			excString += Device::getDeviceTypeString( type );
			excString += "device found for platform ";
			excString += getPlatformParam( mId, CL_PLATFORM_NAME );
			PlatformDeviceExc exc( excString );
			throw exc;
		}
	}
	
	std::vector<cl_device_id> devices( numDevices );
	
	errNum = clGetDeviceIDs( mId, type, numDevices, devices.data(), NULL);
	
	// TODO: Add errNum check here
	
	return devices;
}
	
std::vector<cl_platform_id>& Platform::getAvailablePlatforms()
{
	cl_int errNum;
	cl_uint numPlatforms;
	static std::vector<cl_platform_id> platforms;
	if ( platforms.empty() ) {
		errNum = clGetPlatformIDs( 0, NULL, &numPlatforms );
		
		// TODO: Add errNum check here
		if( errNum != CL_SUCCESS ) {
			throw "could not get Platforms";
		}
		
		platforms.resize( numPlatforms );
		
		errNum = clGetPlatformIDs( numPlatforms, platforms.data(), NULL );
		
		// TODO: Add errNum check here
		if( errNum != CL_SUCCESS ) {
			throw "platforms could not be queried";
		}
	}
	return platforms;
}
	
void Platform::displayPlatformInfo( const PlatformRef &platform )
{
	displayPlatformInfo( platform->getId() );
}

void Platform::displayPlatformInfo( cl_platform_id platformId )
{
	std::cout << platformId;
}
	
std::string Platform::getPlatformParam( cl_platform_id id, cl_platform_info name )
{
	cl_int errNum;
	
	std::size_t paramValueSize;
	
	errNum = clGetPlatformInfo( id, name, 0, NULL, &paramValueSize );
	if (errNum != CL_SUCCESS) {
		//TODO: add detailed errNum string to this
		throw PlatformException( "Failed to find get param from provided platform id." );
	}
	
	std::string info(paramValueSize, ' ');
	
	errNum = clGetPlatformInfo( id, name, paramValueSize, &info[0], NULL );
	
	if (errNum != CL_SUCCESS) {
		std::string exc;
		exc += "Failed to get param info from provided platform id, because ";
		exc += getErrorString( errNum );
		throw PlatformException( exc );
	}
	
	return info;
}
	
std::ostream& operator<<( std::ostream &lhs, const PlatformRef &rhs )
{
	auto platformId = rhs->getId();
	return lhs << platformId;
}
	
std::ostream& operator<<( std::ostream &lhs, const cl_platform_id &platformId )
{
	lhs << "\t" << "CL_PLATFORM_NAME" << ":\t" <<
	Platform::getPlatformParam( platformId, CL_PLATFORM_NAME ) << "\n"
	<< "\t" << "CL_PLATFORM_PROFILE" << ":\t" <<
	Platform::getPlatformParam( platformId, CL_PLATFORM_PROFILE ) << "\n"
	<< "\t" << "CL_PLATFORM_VERSION" << ":\t" <<
	Platform::getPlatformParam( platformId, CL_PLATFORM_VERSION ) << "\n"
	<< "\t" << "CL_PLATFORM_VENDOR" << ":\t" <<
	Platform::getPlatformParam( platformId, CL_PLATFORM_VENDOR ) << "\n"
	<< "\t" << "CL_PLATFORM_EXTENSIONS" << ":\t" <<
	Platform::getPlatformParam( platformId, CL_PLATFORM_EXTENSIONS ) << "\n";
	return lhs;
}

} // namespace cl