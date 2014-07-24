//
//  Image.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "MemoryObj.h"
#include "cinder/Surface.h"

namespace cinder { namespace cl {
	
typedef struct _cl_image_format
{
	cl_channel_order dataFormat;
	cl_channel_type type;
} cl_image_format;

// need to make this an abstract class to forgoe needing a virtual destructor
class ImageBase : public MemoryObj {
public:
	
	struct Format {
		
	};
	
	virtual ~ImageBase();
	
	static void SurfaceChannelOrderToDataFormatAndType( const SurfaceChannelOrder &sco, cl_image_format *format );
	
private:
	ImageBase();
	
	
};

typedef std::shared_ptr<class Image2d> Image2dRef;
	
class Image2d : public ImageBase {
public:
	
	struct Format : public ImageBase::Format {
		
	};
	
	~Image2d();
	
	Image2dRef create( int width, int height, Format format );
	Image2dRef create( const unsigned char *data, int dataFormat, int width, int height, Format format );
	Image2dRef create( const Surface8u &surface, Format format );
	Image2dRef create( const Surface32f &surface, Format format );
	Image2dRef create( const Channel8u &channel, Format format );
	Image2dRef create( const Channel32f &channel, Format format );
	Image2dRef create( ImageSourceRef imageSource, Format format );
	Image2dRef create( GLenum target, GLuint Image2dID, int width, int height, bool doNotDispose );

	
private:
	// These constructors are not protected to allow for shared_ptr's with custom deleters
	/** Consider Image2d::create() instead. Constructs a Image2d of size(\a width, \a height), storing the data in internal format \a aInternalFormat. **/
	Image2d( int width, int height, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d of size(\a width, \a height), storing the data in internal format \a aInternalFormat. Pixel data is provided by \a data and is expected to be interleaved and in format \a dataFormat, for which \c GL_RGB or \c GL_RGBA would be typical values. **/
	Image2d( const unsigned char *data, int dataFormat, int width, int height, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d based on the contents of \a surface. A default value of -1 for \a internalFormat chooses an appropriate internal format automatically. **/
	Image2d( const Surface8u &surface, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d based on the contents of \a surface. A default value of -1 for \a internalFormat chooses an appropriate internal format automatically. **/
	Image2d( const Surface32f &surface, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d based on the contents of \a channel. A default value of -1 for \a internalFormat chooses an appropriate internal format automatically. **/
	Image2d( const Channel8u &channel, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d based on the contents of \a channel. A default value of -1 for \a internalFormat chooses an appropriate internal format automatically. **/
	Image2d( const Channel32f &channel, Format format = Format() );
	/** Consider Image2d::create() instead. Constructs a Image2d based on \a imageSource. A default value of -1 for \a internalFormat chooses an appropriate internal format based on the contents of \a imageSource. **/
	Image2d( const ImageSourceRef &imageSource, Format format = Format() );
	//! Consider Image2d::create() instead. Constructs a Image2d based on an externally initialized OpenGL Image2d. \a aDoNotDispose specifies whether the Image2d is responsible for disposing of the associated OpenGL resource.
	Image2d( GLenum target, GLuint Image2dId, int width, int height, bool doNotDispose );
	//! Consider Image2d::create() instead. Constructs a Image2d based on an externally initialized OpenGL Image2d. \a aDoNotDispose specifies whether the Image2d is responsible for disposing of the associated OpenGL resource.
	Image2d( cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch, void *data );
	
	cl_int width, height;
	
};

class Image3d : public ImageBase {
public:
	
	~Image3d();
	
private:
	
};
	
}}
