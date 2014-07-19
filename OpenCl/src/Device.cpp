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
	// Figure out what to do with this
	// It needs to be like a waterfall for this option
#if defined (__APPLE__) || defined(MACOSX)
	static const char* CL_GL_SHARING_EXT = "cl_APPLE_gl_sharing";
	static const char* CL_GL_EVENT_EXT = "cl_khr_gl_event";
#else
	static const char* CL_GL_SHARING_EXT = "cl_khr_gl_sharing";
#endif
	// Get string containing supported device extensions
	size_t ext_size = 1024;
	char* ext_string = new char[ext_size];
	errNum = clGetDeviceInfo( mId, CL_DEVICE_EXTENSIONS, ext_size, ext_string, &ext_size);
		// Search for GL support in extension string (space delimited)
	bool supported = isExtensionSupported( CL_GL_SHARING_EXT, ext_string, ext_size );
	if( supported )
	{
		// Device supports context sharing with OpenGL
		printf("Found GL Sharing Support!\n");
	}
	supported = false;
	supported = isExtensionSupported( CL_GL_EVENT_EXT, ext_string, ext_size );
	if( supported )
	{
		// Device supports context sharing with OpenGL
		printf("Found GL Sharing Support!\n");
	}
}
	
DeviceRef Device::create( cl_device_id device )
{
	return DeviceRef( new Device( device ) );
}

Device::~Device()
{
	clReleaseDevice(mId);
}

bool Device::isExtensionSupported( const char* support_str, const char* ext_string, size_t ext_buffer_size )
{
	size_t offset = 0;
	const char* space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);
	size_t space_pos = space_substr ? space_substr - ext_string : 0;
	while (space_pos < ext_buffer_size)
	{
		if( strncmp(support_str, ext_string + offset, space_pos) == 0 )
		{
			// Device supports requested extension!
			printf("Info: Found extension support ‘%s’!\n", support_str);
			return 1;
		}
		// Keep searching -- skip to next token string
		offset = space_pos + 1;
		space_substr = strnstr(ext_string + offset, " ", ext_buffer_size - offset);
		space_pos = space_substr ? space_substr - ext_string : 0;
	}
	printf("Warning: Extension not supported ‘%s’!\n", support_str);
	return 0;
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