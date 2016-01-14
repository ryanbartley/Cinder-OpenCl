//
//  Cinder-OpenCL.h
//  HelloWorld
//
//  Created by ryan bartley on 6/12/15.
//
//

#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"
#include "cinder/ImageIo.h"

namespace cinder { namespace ocl {
	
using namespace cl;

cl_context_properties* getDefaultSharedGraphicsContextProperties( const Platform &platform = Platform() );

std::string			errorToString( cl_int error );
std::string			constantToString( cl_int constant );
Program				createProgram( Context &context, const DataSourceRef &dataSource, bool build = true );
	
using ImageFormats = std::vector<ImageFormat>;
	
void				printSupportedImageFormats( const cl::Context &context, cl_mem_object_type type );
ImageFormats&		getSupportedImageFormats( cl::Context context, cl_mem_object_type type );
void				convertChannelOrder( cl_channel_order channel,
										 ImageIo::ColorModel *colorModel,
										 ImageIo::ChannelOrder *channelOrder );
void				convertChannelDataType( cl_channel_type type, ImageIo::DataType *dataType );
cl::ImageFormat		getImageFormat( ImageIo::ChannelOrder channelOrder, ImageIo::DataType dataType );
Image2D				createImage2D( ImageSourceRef imageSource,
								   cl::Context context,
								   cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR );
cl::Buffer			createBuffer( ImageSourceRef imageSource,
								  cl::Context context,
								  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR );
ImageSourceRef		createSource( Image2D image );
	
} // namespace ocl	
	
std::ostream& operator<<( std::ostream &lhs, const ocl::Platform &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Device &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Program &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Kernel &rhs );
	
} // namespace cinder


