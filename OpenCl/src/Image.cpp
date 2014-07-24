//
//  Image.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#include "Image.h"

namespace cinder { namespace cl {
	
void ImageBase::SurfaceChannelOrderToDataFormatAndType( const SurfaceChannelOrder &sco, cl_image_format *format )
{
	switch( sco.getCode() ) {
		case SurfaceChannelOrder::RGB:
			format->dataFormat = CL_RGB;
			format->type = GL_UNSIGNED_BYTE;
		break;
		case SurfaceChannelOrder::RGBA:
		case SurfaceChannelOrder::RGBX:
			format->dataFormat = GL_RGBA;
			format->type = GL_UNSIGNED_BYTE;
		break;
		case SurfaceChannelOrder::BGRA:
		case SurfaceChannelOrder::BGRX:
#if defined( CINDER_GL_ES )
			format->dataFormat = GL_BGRA_EXT;
#else
			format->dataFormat = GL_BGRA;
#endif
			format->type = GL_UNSIGNED_BYTE;
		break;
		default:
			std::cout << "this is an unsupported channel order for a image" << std::endl;
		break;
	}
}
	
	
Image2dRef Image2d::create( int width, int height, Format format )
{
	return Image2dRef( new Image2d( width, height, format ) );
}

Image2dRef Image2d::create( const unsigned char *data, int dataFormat, int width, int height, Format format )
{
	return Image2dRef( new Image2d( data, dataFormat, width, height, format ) );
}

Image2dRef Image2d::create( const Surface8u &surface, Format format )
{
	return Image2dRef( new Image2d( surface, format ) );
}

Image2dRef Image2d::create( const Surface32f &surface, Format format )
{
	return Image2dRef( new Image2d( surface, format ) );
}

Image2dRef Image2d::create( const Channel8u &channel, Format format )
{
	return Image2dRef( new Image2d( channel, format ) );
}

Image2dRef Image2d::create( const Channel32f &channel, Format format )
{
	return Image2dRef( new Image2d( channel, format ) );
}

Image2dRef Image2d::create( ImageSourceRef imageSource, Format format )
{
	return Image2dRef( new Image2d( imageSource, format ) );
}

Image2dRef Image2d::create( GLenum target, GLuint Image2dID, int width, int height, bool doNotDispose )
{
	return Image2dRef( new Image2d( target, Image2dID, width, height, doNotDispose ) );
}
	
//Image2d::Image2d( int width, int height, Format format )
//: mWidth( width ), mHeight( height ),
//mCleanWidth( width ), mCleanHeight( height ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, GL_RGBA );
//	initData( (unsigned char*)0, 0, format.mPixelDataFormat, format.mPixelDataType, format );
//}
//
//Image2d::Image2d( const unsigned char *data, int dataFormat, int width, int height, Format format )
//: mWidth( width ), mHeight( height ),
//mCleanWidth( width ), mCleanHeight( height ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, GL_RGBA );
//	initData( data, 0, dataFormat, GL_UNSIGNED_BYTE, format );
//}
//
//Image2d::Image2d( const Surface8u &surface, Format format )
//: mWidth( surface.getWidth() ), mHeight( surface.getHeight() ),
//mCleanWidth( surface.getWidth() ), mCleanHeight( surface.getHeight() ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, surface.hasAlpha() ? GL_RGBA : GL_RGB );
//	
//	GLint dataFormat;
//	GLenum type;
//	SurfaceChannelOrderToDataFormatAndType( surface.getChannelOrder(), &dataFormat, &type );
//	
//	initData( surface.getData(), surface.getRowBytes() / surface.getChannelOrder().getPixelInc(), dataFormat, type, format );
//}
//
//Image2d::Image2d( const Surface32f &surface, Format format )
//: mWidth( surface.getWidth() ), mHeight( surface.getHeight() ),
//mCleanWidth( surface.getWidth() ), mCleanHeight( surface.getHeight() ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//#if defined( CINDER_GL_ES )
//	initParams( format, surface.hasAlpha() ? GL_RGBA : GL_RGB );
//#else
//	initParams( format, surface.hasAlpha() ? GL_RGBA32F : GL_RGB32F );
//#endif
//	initData( surface.getData(), surface.hasAlpha()?GL_RGBA:GL_RGB, format );
//}
//	
//Image2d::Image2d( const Channel8u &channel, Format format )
//: mWidth( channel.getWidth() ), mHeight( channel.getHeight() ),
//mCleanWidth( channel.getWidth() ), mCleanHeight( channel.getHeight() ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, GL_LUMINANCE );
//	
//	// if the data is not already contiguous, we'll need to create a block of memory that is
//	if( ( channel.getIncrement() != 1 ) || ( channel.getRowBytes() != channel.getWidth() * sizeof( uint8_t ) ) ) {
//		shared_ptr<uint8_t> data( new uint8_t[ channel.getWidth() * channel.getHeight() ], checked_array_deleter<uint8_t>() );
//		uint8_t* dest		= data.get();
//		const int8_t inc	= channel.getIncrement();
//		const int32_t width = channel.getWidth();
//		for ( int y = 0; y < channel.getHeight(); ++y ) {
//			const uint8_t* src = channel.getData( 0, y );
//			for ( int x = 0; x < width; ++x ) {
//				*dest++	= *src;
//				src		+= inc;
//			}
//		}
//		initData( data.get(), channel.getRowBytes() / channel.getIncrement(), GL_LUMINANCE, GL_UNSIGNED_BYTE, format );
//	} else {
//		initData( channel.getData(), channel.getRowBytes() / channel.getIncrement(), GL_LUMINANCE, GL_UNSIGNED_BYTE, format );
//	}
//}
//
//Image2d::Image2d( const Channel32f &channel, Format format )
//: mWidth( channel.getWidth() ), mHeight( channel.getHeight() ),
//mCleanWidth( channel.getWidth() ), mCleanHeight( channel.getHeight() ),
//mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, GL_LUMINANCE );
//	
//	// if the data is not already contiguous, we'll need to create a block of memory that is
//	if( ( channel.getIncrement() != 1 ) || ( channel.getRowBytes() != channel.getWidth() * sizeof(float) ) ) {
//		shared_ptr<float> data( new float[channel.getWidth() * channel.getHeight()], checked_array_deleter<float>() );
//		float* dest			= data.get();
//		const int8_t inc	= channel.getIncrement();
//		const int32_t width = channel.getWidth();
//		for( int y = 0; y < channel.getHeight(); ++y ) {
//			const float* src = channel.getData( 0, y );
//			for( int x = 0; x < width; ++x ) {
//				*dest++ = *src;
//				src		+= inc;
//			}
//		}
//		
//		initData( data.get(), GL_LUMINANCE, format );
//	}
//	else {
//		initData( channel.getData(), GL_LUMINANCE, format );
//	}
//}
//
//Image2d::Image2d( const ImageSourceRef &imageSource, Format format )
//: mWidth( -1 ), mHeight( -1 ), mCleanWidth( -1 ), mCleanHeight( -1 ),
//mFlipped( false )
//{
//	GLint defaultInternalFormat;
//	// Set the internal format based on the image's color space
//	switch( imageSource->getColorModel() ) {
//		case ImageIo::CM_RGB:
//			defaultInternalFormat = ( imageSource->hasAlpha() ) ? GL_RGBA : GL_RGB;
//			break;
//		case ImageIo::CM_GRAY: {
//#if defined( CINDER_GL_ES )
//			defaultInternalFormat = ( imageSource->hasAlpha() ) ? GL_LUMINANCE_ALPHA : GL_LUMINANCE;
//#else
//			defaultInternalFormat = ( imageSource->hasAlpha() ) ?  GL_RG : GL_RED;
//			std::array<int,4> swizzleMask = { GL_RED, GL_RED, GL_RED, GL_GREEN };
//			if( defaultInternalFormat == GL_RED )
//				swizzleMask[3] = GL_ONE;
//				format.setSwizzleMask( swizzleMask );
//#endif
//		} break;
//		default:
//			throw ImageIoExceptionIllegalColorModel( "Unsupported color model for gl::Texture construction." );
//			break;
//	}
//	
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, defaultInternalFormat );
//	initData( imageSource, format );
//}
//
//Image2d::Image2d( GLenum target, GLuint textureId, int width, int height, bool doNotDispose )
//: TextureBase( target, textureId, -1 ), mWidth( width ), mHeight( height ),
//mCleanWidth( width ), mCleanHeight( height ),
//mFlipped( false )
//{
//	mDoNotDispose = doNotDispose;
//	if( mTarget == GL_TEXTURE_2D ) {
//		mMaxU = mMaxV = 1.0f;
//	}
//	else {
//		mMaxU = (float)mWidth;
//		mMaxV = (float)mHeight;
//	}
//}
//
//Image2d::Image2d( const TextureData &data, Format format )
//: mFlipped( false )
//{
//	glGenTextures( 1, &mTextureId );
//	mTarget = format.getTarget();
//	ScopedTextureBind texBindScope( mTarget, mTextureId );
//	initParams( format, 0 /* unused */ );
//	
//	replace( data );
//	
//	if( format.mMipmapping && data.getNumLevels() <= 1 ) {
//#if ! defined( CINDER_GL_ES )
//		glTexParameteri( mTarget, GL_TEXTURE_BASE_LEVEL, format.mBaseMipmapLevel );
//		glTexParameteri( mTarget, GL_TEXTURE_MAX_LEVEL, format.mMaxMipmapLevel );
//#endif		
//		glGenerateMipmap( mTarget );
//	}
//}



	
}}