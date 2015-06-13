//
//  clgl.cpp
//  ProceduralGeometricDisplacement
//
//  Created by ryan bartley on 5/22/15.
//
//

#include "clgl.h"

#include "Context.h"

#include "cinder/gl/BufferObj.h"
#include "cinder/Log.h"
#include "ConstantConversion.h"

namespace cl { namespace gl {

cl::BufferObjRef createBufferFromGl( const ci::gl::BufferObjRef &glBuffer, cl_mem_flags flags )
{
	auto ctx = Context::context();
	if ( ! ctx->isGlShared() ) {
		CI_LOG_E( "Context is not GL shared" );
		return cl::BufferObjRef();
	}
	
	auto ret = cl::BufferObjRef( new cl::BufferObj( flags, glBuffer->getSize() ) );
	
	cl_int errCode;
	
	ret->mId = clCreateFromGLBuffer( ctx->getId(), ret->mFlags, glBuffer->getId(), &errCode );
	
	if( errCode ) {
		CI_LOG_E( "Creating Buffer from GL Buffer " << getErrorString( errCode ) );
		return cl::BufferObjRef();
	}
	
	return ret;
}

} }