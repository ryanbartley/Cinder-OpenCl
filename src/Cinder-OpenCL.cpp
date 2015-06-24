//
//  Cinder-OpenCL.cpp
//  HelloWorld
//
//  Created by ryan bartley on 6/12/15.
//
//

#if defined( CINDER_MAC )
#include <OpenGL/OpenGL.h>
#else
#include <Windows.h>
#include <GL\GL.h>
#endif
#include "Cinder-OpenCL.h"
#include "cinder/Utilities.h"


namespace cinder {
	
std::string errorToString( cl_int error )
{
	switch( error ) {
		case CL_SUCCESS: return "CL_SUCCESS";
		case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
		case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
		case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
		case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
		case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
		case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
		case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
		case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
		case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
		case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
		case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
		case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
		case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
		case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
		case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
		case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
		case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
		case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
		case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
		case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
		case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
		case CL_INVALID_PROGRAM_EXECUTABLE: return "CL_INVALID_PROGRAM_EXECUTABLE";
		case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
		case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
		case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
		case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
		case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
		case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
		case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
		case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
		case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
		case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
		case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
		case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
		case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
		case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
		case CL_INVALID_IMAGE_DESCRIPTOR: return "CL_INVALID_IMAGE_DESCRIPTOR";
		case CL_INVALID_COMPILER_OPTIONS: return "CL_INVALID_COMPILER_OPTIONS";
		case CL_INVALID_LINKER_OPTIONS: return "CL_INVALID_LINKER_OPTIONS";
		case CL_INVALID_DEVICE_PARTITION_COUNT: return "CL_INVALID_DEVICE_PARTITION_COUNT";
		case CL_INVALID_PIPE_SIZE: return "CL_INVALID_PIPE_SIZE";
		case CL_INVALID_DEVICE_QUEUE: return "CL_INVALID_DEVICE_QUEUE";
		default: return "Unknown Error";
	}
}

std::string constantToString( cl_int constant )
{
	switch( constant ) {
		case CL_R: return "CL_R";
		case CL_Rx: return "CL_Rx";
		case CL_A: return "CL_A";
		case CL_INTENSITY: return "CL_INTENSITY";
		case CL_RG: return "CL_RG";
		case CL_RGx: return "CL_RGx";
		case CL_RA: return "CL_RA";
		case CL_RGB: return "CL_RGB";
		case CL_RGBx: return "CL_RGBx";
		case CL_RGBA: return "CL_RGBA";
		case CL_BGRA: return "CL_BGRA";
		case CL_ARGB: return "CL_ARGB";
		case CL_LUMINANCE: return "CL_LUMINANCE";
		case CL_SNORM_INT8: return "CL_SNORM_INT8";
		case CL_SNORM_INT16: return "CL_SNORM_INT16";
		case CL_UNORM_INT8: return "CL_UNORM_INT8";
		case CL_UNORM_INT16: return "CL_UNORM_INT16";
		case CL_SIGNED_INT8: return "CL_SIGNED_INT8";
		case CL_SIGNED_INT16: return "CL_SIGNED_INT16";
		case CL_SIGNED_INT32: return "CL_SIGNED_INT32";
		case CL_UNSIGNED_INT8: return "CL_UNSIGNED_INT8";
		case CL_UNSIGNED_INT16: return "CL_UNSIGNED_INT16";
		case CL_UNSIGNED_INT32: return "CL_UNSIGNED_INT32";
		case CL_HALF_FLOAT: return "CL_HALF_FLOAT";
		case CL_FLOAT: return "CL_FLOAT";
		case CL_UNORM_SHORT_565: return "CL_UNORM_SHORT_565";
		case CL_UNORM_SHORT_555: return "CL_UNORM_SHORT_555";
		case CL_UNORM_INT_101010: return "CL_UNORM_INT_101010";
	}
}

std::ostream& operator<<( std::ostream &lhs, const cl::Platform &rhs )
{
	lhs << "CL_PLATFORM_NAME:   " << rhs.getInfo<CL_PLATFORM_NAME>() << std::endl;
	lhs << "CL_PLATFORM_VENDOR: " << rhs.getInfo<CL_PLATFORM_VENDOR>() << std::endl;
	lhs << "CL_PLATFORM_VERSION:" << rhs.getInfo<CL_PLATFORM_VERSION>() << std::endl;
	lhs	<< "CL_PLATFORM_PROFILE:"	<< rhs.getInfo<CL_PLATFORM_PROFILE>() << std::endl;
	auto extensions = std::string( rhs.getInfo<CL_PLATFORM_EXTENSIONS>() );
	auto extensionList = split( extensions, ' ' );
	int i = 0;
	lhs << "CL_PLATFORM_EXTENSIONS:\t" << extensionList[i++] << std::endl;
	for( ; i < extensionList.size(); i++ ) {
		lhs << "                    \t" << extensionList[i] << std::endl;
	}
	return lhs;
}
	
std::ostream& operator<<( std::ostream &lhs, const cl::Device &rhs )
{
	lhs << "CL_DEVICE_NAME:      " << rhs.getInfo<CL_DEVICE_NAME>() << std::endl;
	lhs << "CL_DEVICE_VENDOR:    " << rhs.getInfo<CL_DEVICE_VENDOR>() << std::endl;
	lhs << "CL_DRIVER_VERSION:   " << rhs.getInfo<CL_DRIVER_VERSION>() << std::endl;
	lhs << "CL_DEVICE_PROFILE1:  " << rhs.getInfo<CL_DEVICE_PROFILE>() << std::endl;
	lhs << "CL_DEVICE_VERSION:   " << rhs.getInfo<CL_DEVICE_VERSION>() << std::endl;
	lhs	<< "CL_DEVICE_MAX_COMPUTE_UNITS:" << rhs.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
	auto extensions = std::string( rhs.getInfo<CL_DEVICE_EXTENSIONS>() );
	auto extensionList = split( extensions, ' ' );
	int i = 0;
	lhs << "CL_DEVICE_EXTENSIONS:" << extensionList[i++] << std::endl;
	for( ; i < extensionList.size(); i++ ) {
		lhs << "                    " << extensionList[i] << std::endl;
	}
	return lhs;
}
	
cl_context_properties* getDefaultSharedGraphicsContextProperties( cl::Platform platform )
{
#if defined (__APPLE__) || defined(MACOSX)
	static cl_context_properties contextProperties[] = { 0, 0, 0 };
	contextProperties[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
	contextProperties[1] = (cl_context_properties)CGLGetShareGroup( ::CGLGetCurrentContext() ) ;
#else
	static cl_context_properties contextProperties[] = { 0, 0, 0, 0, 0, 0, 0 };
	contextProperties[0] = CL_GL_CONTEXT_KHR;
	contextProperties[1] = (cl_context_properties) wglGetCurrentContext();
	contextProperties[2] = CL_WGL_HDC_KHR;
	contextProperties[3] = (cl_context_properties) wglGetCurrentDC();
	contextProperties[4] = CL_CONTEXT_PLATFORM;
	contextProperties[5] = ( cl_context_properties ) platform();
#endif
	//TODO: DirectX Implementation
	return contextProperties;
}
	
void convertChannelOrder( cl_channel_order clChannelOrder, ImageIo::ColorModel *colorModel, ImageIo::ChannelOrder *channelOrder )
{
	switch ( clChannelOrder ) {
		case CL_R:
			*colorModel = ImageIo::CM_GRAY; *channelOrder = ImageIo::Y;
		break;
		case CL_RGB:
			*colorModel = ImageIo::CM_RGB; *channelOrder = ImageIo::RGB;
		break;
		case CL_RGBA:
			*colorModel = ImageIo::CM_RGB; *channelOrder = ImageIo::RGBA;
		break;
		default:
			throw ImageIoException();
	}
}

void convertChannelDataType( cl_channel_type type, ImageIo::DataType *dataType )
{
	switch ( type ) {
		case CL_UNSIGNED_INT8:
			*dataType = ImageIo::DataType::UINT8;
		break;
		case CL_UNSIGNED_INT16:
			*dataType = ImageIo::DataType::UINT16;
		break;
		case CL_FLOAT:
			*dataType = ImageIo::DataType::FLOAT32;
		break;
		default:
			throw ImageIoExceptionIllegalDataType();
	}
}
	
	
}