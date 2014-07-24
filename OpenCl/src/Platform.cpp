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
	
PlatformRef Platform::create( cl_platform_id platform, bool cacheAllDevices )
{
	auto ret = PlatformRef( new Platform( platform ) );
	if( cacheAllDevices ) {
		ret->cacheDevices();
	}
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
		exc += getClErrorString( errNum );
		throw PlatformException( exc );
	}
	
	return info;
}
	
const char * Platform::getClErrorString(cl_int err){
	switch(err){
		case 0: return "CL_SUCCESS";
		case -1: return "CL_DEVICE_NOT_FOUND";
		case -2: return "CL_DEVICE_NOT_AVAILABLE";
		case -3: return "CL_COMPILER_NOT_AVAILABLE";
		case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case -5: return "CL_OUT_OF_RESOURCES";
		case -6: return "CL_OUT_OF_HOST_MEMORY";
		case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case -8: return "CL_MEM_COPY_OVERLAP";
		case -9: return "CL_IMAGE_FORMAT_MISMATCH";
		case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case -11: return "CL_BUILD_PROGRAM_FAILURE";
		case -12: return "CL_MAP_FAILURE";
			
		case -30: return "CL_INVALID_VALUE";
		case -31: return "CL_INVALID_DEVICE_TYPE";
		case -32: return "CL_INVALID_PLATFORM";
		case -33: return "CL_INVALID_DEVICE";
		case -34: return "CL_INVALID_CONTEXT";
		case -35: return "CL_INVALID_QUEUE_PROPERTIES";
		case -36: return "CL_INVALID_COMMAND_QUEUE";
		case -37: return "CL_INVALID_HOST_PTR";
		case -38: return "CL_INVALID_MEM_OBJECT";
		case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case -40: return "CL_INVALID_IMAGE_SIZE";
		case -41: return "CL_INVALID_SAMPLER";
		case -42: return "CL_INVALID_BINARY";
		case -43: return "CL_INVALID_BUILD_OPTIONS";
		case -44: return "CL_INVALID_PROGRAM";
		case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case -46: return "CL_INVALID_KERNEL_NAME";
		case -47: return "CL_INVALID_KERNEL_DEFINITION";
		case -48: return "CL_INVALID_KERNEL";
		case -49: return "CL_INVALID_ARG_INDEX";
		case -50: return "CL_INVALID_ARG_VALUE";
		case -51: return "CL_INVALID_ARG_SIZE";
		case -52: return "CL_INVALID_KERNEL_ARGS";
		case -53: return "CL_INVALID_WORK_DIMENSION";
		case -54: return "CL_INVALID_WORK_GROUP_SIZE";
		case -55: return "CL_INVALID_WORK_ITEM_SIZE";
		case -56: return "CL_INVALID_GLOBAL_OFFSET";
		case -57: return "CL_INVALID_EVENT_WAIT_LIST";
		case -58: return "CL_INVALID_EVENT";
		case -59: return "CL_INVALID_OPERATION";
		case -60: return "CL_INVALID_GL_OBJECT";
		case -61: return "CL_INVALID_BUFFER_SIZE";
		case -62: return "CL_INVALID_MIP_LEVEL";
		case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
		default: return "Unknown OpenCL error";
	}
}


PlatformException::PlatformException( const std::string &message ) throw()
: Exception()
{
	strncpy( mMessage, message.c_str(), 255 );
}

}}