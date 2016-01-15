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

using namespace ci;
using namespace ci::app;
using namespace std;


namespace cinder { namespace ocl {
	
std::vector<cl::ImageFormat>& getSupportedImageFormats( cl::Context context, cl_mem_object_type type )
{
	using ImageFormatCache = std::map<cl_mem_object_type, std::vector<cl::ImageFormat>>;
	using ContextImageFormatCache = std::map<cl_context, ImageFormatCache>;
	static ContextImageFormatCache contextImageFormatCache;
	
	ContextImageFormatCache::iterator conIt = contextImageFormatCache.find( context() );
	if( conIt == contextImageFormatCache.end() ) {
		auto createdCache = contextImageFormatCache.insert( { context(), ImageFormatCache() } );
		conIt = createdCache.first;
	}
	
	ImageFormatCache::iterator imageIt = conIt->second.find( type );
	if( imageIt == conIt->second.end() ) {
		std::vector<cl::ImageFormat> imageFormats;
		context.getSupportedImageFormats( CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, type, &imageFormats );
		auto createdImageFormat = conIt->second.insert( { type, move( imageFormats ) } );
		imageIt = createdImageFormat.first;
	}
	return imageIt->second;
}
	
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
//		case CL_INVALID_PIPE_SIZE: return "CL_INVALID_PIPE_SIZE";
//		case CL_INVALID_DEVICE_QUEUE: return "CL_INVALID_DEVICE_QUEUE";
		default: return "Unknown Constant";
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
#if defined (__APPLE__) || defined( MACOSX )
		case CL_ABGR_APPLE: return "CL_ABGR_APPLE";
#endif
		case CL_ARGB: return "CL_ARGB";
		case CL_LUMINANCE: return "CL_LUMINANCE";
		case CL_DEPTH: return "CL_DEPTH";
		case CL_DEPTH_STENCIL: return "CL_DEPTH_STENCIL";
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
		case CL_KERNEL_ARG_ADDRESS_GLOBAL: return "CL_KERNEL_ARG_ADDRESS_GLOBAL";
		case CL_KERNEL_ARG_ADDRESS_LOCAL: return "CL_KERNEL_ARG_ADDRESS_LOCAL";
		case CL_KERNEL_ARG_ADDRESS_CONSTANT: return "CL_KERNEL_ARG_ADDRESS_CONSTANT";
		case CL_KERNEL_ARG_ADDRESS_PRIVATE: return "CL_KERNEL_ARG_ADDRESS_PRIVATE";
		case CL_KERNEL_ARG_ACCESS_READ_ONLY: return "CL_KERNEL_ARG_ACCESS_READ_ONLY";
		case CL_KERNEL_ARG_ACCESS_WRITE_ONLY: return "CL_KERNEL_ARG_ACCESS_WRITE_ONLY";
		case CL_KERNEL_ARG_ACCESS_READ_WRITE: return "CL_KERNEL_ARG_ACCESS_READ_WRITE";
		case CL_KERNEL_ARG_ACCESS_NONE: return "CL_KERNEL_ARG_ACCESS_NONE";
		default: return errorToString( constant );
	}
}
	
void printSupportedImageFormats( const cl::Context &context, cl_mem_object_type type )
{
	auto & imageFormats = getSupportedImageFormats( context, type );
	
	for( auto & imageFormat : imageFormats ) {
		console() << "Channel Order: " << ocl::constantToString( imageFormat.image_channel_order )
		<< " Channel Data Type: " << constantToString( imageFormat.image_channel_data_type ) << std::endl;
	}
}
	
Program createProgram( Context &context, const DataSourceRef &dataSource, bool build )
{
	return Program( context, loadString( dataSource ), build );
}
	
cl_context_properties* getDefaultSharedGraphicsContextProperties( const cl::Platform &platform )
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
		case CL_R: *colorModel = ImageIo::CM_GRAY; *channelOrder = ImageIo::Y; break;
		case CL_RGB: *colorModel = ImageIo::CM_RGB; *channelOrder = ImageIo::RGB; break;
		case CL_RGBA: *colorModel = ImageIo::CM_RGB; *channelOrder = ImageIo::RGBA; break;
		default: throw ImageIoExceptionIllegalChannelOrder();
	}
}

void convertChannelDataType( cl_channel_type type, ImageIo::DataType *dataType )
{
	switch ( type ) {
		case CL_UNORM_INT8: *dataType = ImageIo::DataType::UINT8; break;
		case CL_UNSIGNED_INT8: *dataType = ImageIo::DataType::UINT8; break;
		case CL_UNSIGNED_INT16: *dataType = ImageIo::DataType::UINT16; break;
		case CL_HALF_FLOAT: *dataType = ImageIo::DataType::FLOAT16; break;
		case CL_FLOAT: *dataType = ImageIo::DataType::FLOAT32; break;
		default: throw ImageIoExceptionIllegalDataType();
	}
}
	
cl::ImageFormat getImageFormat( ImageIo::ChannelOrder channelOrder, ImageIo::DataType dataType )
{
	cl::ImageFormat ret;
	
	switch ( channelOrder ) {
		case ImageIo::ChannelOrder::RGBA: ret.image_channel_order = CL_RGBA; break;
		case ImageIo::ChannelOrder::RGB: ret.image_channel_order = CL_RGB; break;
		case ImageIo::ChannelOrder::Y: ret.image_channel_order = CL_R; break;
		case ImageIo::ChannelOrder::YA: ret.image_channel_order = CL_RA; break;
		case ImageIo::ChannelOrder::BGRA: ret.image_channel_order = CL_BGRA; break;
		case ImageIo::ChannelOrder::ARGB: ret.image_channel_order = CL_ARGB; break;
		default: throw ImageIoExceptionIllegalChannelOrder(); break;
	}
	
	switch ( dataType ) {
		case ImageIo::DataType::UINT8: ret.image_channel_data_type = CL_UNORM_INT8; break;
		case ImageIo::DataType::UINT16: ret.image_channel_data_type = CL_UNSIGNED_INT16; break;
		// TODO: Test this format.
		case ImageIo::DataType::FLOAT16: ret.image_channel_data_type = CL_HALF_FLOAT; break;
		case ImageIo::DataType::FLOAT32: ret.image_channel_data_type = CL_FLOAT; break;
		default: throw ImageIoExceptionIllegalDataType(); break;
	}

	return ret;
}
	
class ImageTargetClImage : public ImageTarget {
public:
	static std::shared_ptr<ImageTargetClImage> create( Image2D *image, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha );
	
	bool	hasAlpha() const override { return mHasAlpha; }
	void*	getRowPointer( int32_t row ) override;
	void	finalize() override;
	
private:
	ImageTargetClImage( Image2D *image, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha );
	
	Image2D			*mImage;
	cl::Context		mContext;
	cl_mem_flags	mFlags;
	
	ci::ivec2		mSize;
	
	unique_ptr<uint8_t[]>	mDataStore; // may be NULL
	
	int32_t		mRowInc;
	bool		mHasAlpha;
	uint8_t		mPixelInc, mNumBytesPerPixel;
};

std::shared_ptr<ImageTargetClImage> ImageTargetClImage::create( Image2D *image, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha )
{
	return std::shared_ptr<ImageTargetClImage>( new ImageTargetClImage( image, context, flags, size, channelOrder, dataType, isGray, hasAlpha ) );
}

ImageTargetClImage::ImageTargetClImage( Image2D *image, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha )
: mImage( image ), mContext( context ), mFlags( flags ), mSize( size ), mNumBytesPerPixel( 1 )
{
	setDataType( dataType );
	
	if( isGray ) {
		mPixelInc = mHasAlpha ? 2 : 1;
	}
	else {
		mPixelInc = mHasAlpha ? 4 : 3;
	}
	mRowInc = mSize.x * mPixelInc;
	
	switch ( dataType) {
		case ImageIo::UINT8: mNumBytesPerPixel = 1; break;
		case ImageIo::UINT16: mNumBytesPerPixel = 2; break;
		case ImageIo::FLOAT16: mNumBytesPerPixel = 2; break;
		case ImageIo::FLOAT32: mNumBytesPerPixel = 4; break;
		case ImageIo::DATA_UNKNOWN: throw ImageIoExceptionIllegalDataType(); break;
	}
	
	mDataStore = std::unique_ptr<uint8_t[]>( new uint8_t[mSize.y * mRowInc * mNumBytesPerPixel] );
	
	setChannelOrder( channelOrder );
	setColorModel( isGray ? ImageIo::CM_GRAY : ImageIo::CM_RGB );
}
	
void* ImageTargetClImage::getRowPointer( int32_t row )
{
	return mDataStore.get() + (row * mRowInc * mNumBytesPerPixel);
}
	
void ImageTargetClImage::finalize()
{
	auto imageFormat = getImageFormat( getChannelOrder(), getDataType() );
	*mImage = Image2D( mContext, mFlags, imageFormat, mSize.x, mSize.y, mRowInc * mNumBytesPerPixel, mDataStore.get() );
}

class ImageTargetCLBuffer : public ImageTarget {
public:
	static std::shared_ptr<ImageTargetCLBuffer> create( cl::Buffer *buffer, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha );
	
	bool	hasAlpha() const override { return mHasAlpha; }
	void*	getRowPointer( int32_t row ) override;
	void	finalize() override;
	
private:
	ImageTargetCLBuffer( cl::Buffer *buffer, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha );
	
	cl::Buffer		*mBuffer;
	cl::Context		mContext;
	cl_mem_flags	mFlags;
	
	ci::ivec2		mSize;
	
	unique_ptr<uint8_t[]>	mDataStore; // may be NULL
	
	int32_t		mRowInc;
	bool		mHasAlpha;
	uint8_t		mPixelInc, mNumBytesPerPixel;
};

std::shared_ptr<ImageTargetCLBuffer> ImageTargetCLBuffer::create( cl::Buffer *buffer, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha )
{
	return std::shared_ptr<ImageTargetCLBuffer>( new ImageTargetCLBuffer( buffer, context, flags, size, channelOrder, dataType, isGray, hasAlpha ) );
}

ImageTargetCLBuffer::ImageTargetCLBuffer( cl::Buffer *buffer, Context context, cl_mem_flags flags, const ci::ivec2 &size, ImageIo::ChannelOrder &channelOrder, ImageIo::DataType dataType, bool isGray, bool hasAlpha )
: mBuffer( buffer ), mContext( context ), mFlags( flags ), mSize( size ), mNumBytesPerPixel( 1 )
{
	setDataType( dataType );
	
	if( isGray ) {
		mPixelInc = mHasAlpha ? 2 : 1;
	}
	else {
		mPixelInc = mHasAlpha ? 4 : 3;
	}
	mRowInc = mSize.x * mPixelInc;
	
	switch ( dataType) {
		case ImageIo::UINT8: mNumBytesPerPixel = 1; break;
		case ImageIo::UINT16: mNumBytesPerPixel = 2; break;
		case ImageIo::FLOAT16: mNumBytesPerPixel = 2; break;
		case ImageIo::FLOAT32: mNumBytesPerPixel = 4; break;
		case ImageIo::DATA_UNKNOWN: throw ImageIoExceptionIllegalDataType(); break;
	}
	
	mDataStore = std::unique_ptr<uint8_t[]>( new uint8_t[mSize.y * mRowInc * mNumBytesPerPixel] );
	
	setChannelOrder( channelOrder );
	setColorModel( isGray ? ImageIo::CM_GRAY : ImageIo::CM_RGB );
}

void* ImageTargetCLBuffer::getRowPointer( int32_t row )
{
	return mDataStore.get() + (row * mRowInc * mNumBytesPerPixel);
}

void ImageTargetCLBuffer::finalize()
{
	*mBuffer = cl::Buffer( mContext, mFlags, mRowInc * mNumBytesPerPixel * mSize.y, mDataStore.get() );
}
	
class ImageSourceCl : public ImageSource {
public:
	ImageSourceCl( Image2D image )
	: ImageSource()
	{
		mWidth = image.getImageInfo<CL_IMAGE_WIDTH>();
		mHeight = image.getImageInfo<CL_IMAGE_HEIGHT>();
		auto imageFormat = image.getImageInfo<CL_IMAGE_FORMAT>();
		
		// Convert the color model and channel order
		ColorModel colorModel; ChannelOrder channelOrder;
		convertChannelOrder( imageFormat.image_channel_order, &colorModel, &channelOrder );
		setColorModel( colorModel );
		setChannelOrder( channelOrder );
		
		// Convert the data type
		DataType dataType;
		convertChannelDataType( imageFormat.image_channel_data_type, &dataType );
		setDataType( dataType );
		
		mRowBytes = image.getImageInfo<CL_IMAGE_ROW_PITCH>();
		
		cl::size_t<3> offset;
		cl::size_t<3> region;
		region[0] = mWidth;
		region[1] = mHeight;
		region[2] = 1; // it'll break if this isn't true
		mData.reset( new uint8_t[ mWidth * mHeight *
							  dataTypeBytes( mDataType ) *
							  channelOrderNumChannels( mChannelOrder ) ] );
		
		auto context = image.getInfo<CL_MEM_CONTEXT>();
		cl::CommandQueue queue( context );
		
		queue.enqueueReadImage( image, CL_BLOCKING, offset, region, mRowBytes, 0, mData.get() );
	}
	
	void load( ImageTargetRef target ) override {
		// get a pointer to the ImageSource function appropriate for handling our data configuration
		ImageSource::RowFunc func = setupRowFunc( target );
	
		const uint8_t *data = mData.get();
		for( int32_t row = 0; row < mHeight; ++row ) {
			((*this).*func)( target, row, data );
			data += mRowBytes;
		}
	}
	
private:
	
	std::unique_ptr<uint8_t[]>	mData;
	uint32_t					mRowBytes;
};
	
Image2D createImage2D( const ImageSourceRef &imageSource, cl::Context context, cl_mem_flags flags )
{
	cl::Image2D image;
	// setup an appropriate dataFormat/ImageTargetTexture based on the image's color space
	ImageIo::ChannelOrder channelOrder;
	bool isGray = false;
	switch( imageSource->getColorModel() ) {
		case ImageSource::CM_RGB:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::RGBA : ImageIo::RGB;
			break;
		case ImageSource::CM_GRAY:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::YA : ImageIo::Y;
			isGray = true;
			break;
		default: // if this is some other color space, we'll have to punt and go w/ RGB
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::RGBA : ImageIo::RGB;
			break;
	}
	auto imageSize = ivec2( imageSource->getWidth(), imageSource->getHeight() );
	auto target = ImageTargetClImage::create( &image, context, flags, imageSize, channelOrder, imageSource->getDataType(), isGray, imageSource->hasAlpha() );
	imageSource->load( target );
	target->finalize();
	return image;
}
	
cl::Buffer createBuffer( const ImageSourceRef &imageSource, cl::Context context, cl_mem_flags flags )
{
	cl::Buffer ret;
	// setup an appropriate dataFormat/ImageTargetTexture based on the image's color space
	ImageIo::ChannelOrder channelOrder;
	bool isGray = false;
	switch( imageSource->getColorModel() ) {
		case ImageSource::CM_RGB:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::RGBA : ImageIo::RGB;
			break;
		case ImageSource::CM_GRAY:
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::YA : ImageIo::Y;
			isGray = true;
			break;
		default: // if this is some other color space, we'll have to punt and go w/ RGB
			channelOrder = ( imageSource->hasAlpha() ) ? ImageIo::RGBA : ImageIo::RGB;
			break;
	}
	auto imageSize = ivec2( imageSource->getWidth(), imageSource->getHeight() );
	auto target = ImageTargetCLBuffer::create( &ret, context, flags, imageSize, channelOrder, imageSource->getDataType(), isGray, imageSource->hasAlpha() );
	imageSource->load( target );
	target->finalize();
	return ret;
}
	
cl::ImageGL createImageGL( const gl::Texture2dRef &texture, cl::Context context, cl_mem_flags flags )
{
	return ImageGL( context, flags, texture->getTarget(), 0, texture->getId() );
}
	
ImageSourceRef createSource( Image2D image )
{
	return ImageSourceRef( new ImageSourceCl( image ) );
}
	
	
} // namespace ocl
	
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

std::ostream& operator<<( std::ostream &lhs, const cl::Program &rhs )
{
	lhs << "PROGRAM SOURCE: " << std::endl;
	lhs << rhs.getInfo<CL_PROGRAM_SOURCE>() << std::endl;
	return lhs;
}

std::ostream& operator<<( std::ostream &lhs, const cl::Kernel &rhs )
{
	lhs << "KERNEL NAME:	" << rhs.getInfo<CL_KERNEL_FUNCTION_NAME>() << endl;
	auto numArgs = rhs.getInfo<CL_KERNEL_NUM_ARGS>();
	lhs << "NUM ARGUMENTS	" << numArgs << endl;
	for( int i = 0; i < numArgs; i++ ) {
		lhs << "ARG " << i << ":" << endl;
		
		auto typeName = rhs.getArgInfo<CL_KERNEL_ARG_TYPE_NAME>( i );
		auto name = rhs.getArgInfo<CL_KERNEL_ARG_NAME>( i );
		if( typeName.empty() && name.empty() ) {
			lhs << "\t Arg Type and Name not available, to see Type and Name,"
			"pass -cl-kernel-arg-info option in to ocl::Program::build." << endl;
		}
		else {
			lhs << "\t ARG: " << typeName << " " << name << endl;
		}
		
		auto addQualifier = rhs.getArgInfo<CL_KERNEL_ARG_ADDRESS_QUALIFIER>( i );
		auto accQualifier = rhs.getArgInfo<CL_KERNEL_ARG_ACCESS_QUALIFIER>( i );
		
		lhs << "\t ADDRESS QUALIFIER: " << ocl::constantToString( addQualifier ) << endl;
		if( accQualifier != CL_KERNEL_ARG_ACCESS_NONE )
			lhs << "\t ACCESS QUALIFIER: " << ocl::constantToString( accQualifier ) << endl;
	}
	return lhs;
}

} // namespace cinder