//
//  clgl.h
//  ProceduralGeometricDisplacement
//
//  Created by ryan bartley on 5/22/15.
//
//

#include "BufferObj.h"

namespace cl { namespace gl {
	
cl::BufferObjRef createBufferFromGl( const ci::gl::BufferObjRef &glBuffer, cl_mem_flags flags );
	
} } // namespace gl // namespace cl