//
//  MarchingCubes.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "MarchingCubes.h"
#include "CommandQueue.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace ci::cl;

const int VERT_POSITION_INDEX = 0;
const int VERT_NORMAL_INDEX = 1;
const int POINT_COLOR_INDEX = 2;
const int POINT_POSITION_INDEX = 3;

size_t full_size = width * height * depth;

MarchingCubes::MarchingCubes( const ci::cl::CommandQueueRef &commandQueue )
: mCommandQueue( commandQueue )
{
	std::vector<vec4> vertPosData(MAX_VERTS);
	std::vector<vec4> vertNormData(MAX_VERTS);
	std::vector<vec4> pointColorData(full_size);
	std::vector<vec4> pointData(full_size);

	int x, y, z;
	for(x = 0; x < width; x++)
	for(y = 0; y < height; y++)
	for(z = 0; z < depth; z++) {
		int id = x + y * width + z * width * height;
		vec4 position = vec4(x, y, z, 1);
		pointData[id] = position;
	}

	mGlVertPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertPosData.data(), GL_DYNAMIC_COPY);
	mGlVertNormals = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertNormData.data(), GL_DYNAMIC_COPY);
	mGlPointColors = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointColorData.data(), GL_DYNAMIC_COPY);
	mGlPointPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointData.data(), GL_STATIC_DRAW);
	
	mVao = gl::Vao::create();
	{
		gl::ScopedVao scopeVao( mVao );
		{
			gl::ScopedBuffer scopeBuffer( mGlVertPositions );
			gl::enableVertexAttribArray( VERT_POSITION_INDEX );
			gl::vertexAttribPointer( VERT_POSITION_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
		{
			gl::ScopedBuffer scopeBuffer( mGlVertNormals );
			gl::enableVertexAttribArray( VERT_NORMAL_INDEX );
			gl::vertexAttribPointer( VERT_NORMAL_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
		{
			gl::ScopedBuffer scopeBuffer( mGlPointColors );
			gl::enableVertexAttribArray( POINT_COLOR_INDEX );
			gl::vertexAttribPointer( POINT_COLOR_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
		{
			gl::ScopedBuffer scopeBuffer( mGlPointPositions );
			gl::enableVertexAttribArray( POINT_POSITION_INDEX );
			gl::vertexAttribPointer( POINT_POSITION_INDEX, 4, GL_FLOAT, GL_FALSE, 0, nullptr );
		}
	}
	
	mRenderGlsl = gl::GlslProg::create( gl::GlslProg::Format() );
	mShadowGlsl = gl::GlslProg::create( gl::GlslProg::Format() );
	
	
	mClPointColors = cl::BufferObj::create( mGlPointColors, CL_MEM_READ_WRITE );
	mClVolume = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(float) * full_size, nullptr );
	mClVertPositions = cl::BufferObj::create( mGlVertPositions, CL_MEM_READ_WRITE );
	mClVertNormals = cl::BufferObj::create( mGlVertNormals, CL_MEM_READ_WRITE );
	mClVertIndex = cl::BufferObj::create( CL_MEM_READ_WRITE, sizeof(int), nullptr );
	
	
	mClProgram = cl::Program::create( loadAsset( "marching_cubes.cl" ) );
	
	mKernWritePoint = mClProgram->createKernel( "write_point" );
	mKernWritePoint->setKernelArg( 0, mClVolume );
	mKernWritePoint->setKernelArg( 4, sizeof(int), (void*)&width );
	mKernWritePoint->setKernelArg( 5, sizeof(int), (void*)&height );
	mKernWritePoint->setKernelArg( 6, sizeof(int), (void*)&depth );
	
	mKernWriteMetaball = mClProgram->createKernel( "write_metaball" );
	mKernWriteMetaball->setKernelArg( 0, mClVolume );
	
	mKernWriteMetaballs = mClProgram->createKernel( "write_metaballs" );
	mKernWriteMetaballs->setKernelArg( 0, mClVolume );
	
	mKernWriteClear = mClProgram->createKernel( "write_clear" );
	mKernWriteClear->setKernelArg( 0, mClVolume );
	
	mKernWritePointColorBack = mClProgram->createKernel( "write_point_color_back" );
	mKernWritePointColorBack->setKernelArg( 0, mClVolume );
	mKernWritePointColorBack->setKernelArg( 1, mClPointColors );

	mKernConstructSurface = mClProgram->createKernel( "construct_surface" );
	mKernConstructSurface->setKernelArg( 0, mClVolume );
	
	mKernGenNormals = mClProgram->createKernel( "generate_flat_normals" );
	
	mKernGenNormalsSmooth = mClProgram->createKernel( "generate_smoot_normals" );
}

void MarchingCubes::clear()
{
	mCommandQueue->NDRangeKernel( mKernWriteClear, 1, nullptr, &full_size, nullptr );
}

void MarchingCubes::point( const ivec4 &point ) {

	if ((point.x >= width) || (point.y >= height) || (point.z >= depth) || (point.x < 0) || (point.y < 0) || (point.z < 0)) {
		CI_LOG_E("Point outside of volume");
	}

	mKernWritePoint->setKernelArg( 1, sizeof(int), (void*)&point.x );
	mKernWritePoint->setKernelArg( 2, sizeof(int), (void*)&point.y );
	mKernWritePoint->setKernelArg( 3, sizeof(int), (void*)&point.z );
	mKernWritePoint->setKernelArg( 7, sizeof(float), (void*)&point.w );
	static size_t work_size = 1;
	mCommandQueue->NDRangeKernel( mKernWritePoint, 1, nullptr, &work_size, nullptr );
}

void MarchingCubes::metaball( const ci::vec3 &pos ) {
	const int METABALL_SIZE = 10;

	int bot_x = math<int>::max(floor(pos.x) - METABALL_SIZE, 0);
	int bot_y = math<int>::max(floor(pos.y) - METABALL_SIZE, 0);
	int bot_z = math<int>::max(floor(pos.z) - METABALL_SIZE, 0);

	int top_x = math<int>::min(ceil(pos.x) + METABALL_SIZE, width-1);
	int top_y = math<int>::min(ceil(pos.y) + METABALL_SIZE, height-1);
	int top_z = math<int>::min(ceil(pos.z) + METABALL_SIZE, depth-1);

	size_t count = (top_x - bot_x) * (top_y - bot_y) * (top_z - bot_z);

	ivec3 bottom{bot_x, bot_y, bot_z};
	ivec3 top	{top_x, top_y, top_z};
	ivec3 size	{width, height, depth};

	mKernWriteMetaball->setKernelArg( 1, sizeof(ivec3), &bottom );
	mKernWriteMetaball->setKernelArg( 2, sizeof(ivec3), &top);
	mKernWriteMetaball->setKernelArg( 3, sizeof(ivec3), &size);
	mKernWriteMetaball->setKernelArg( 4, sizeof(float), (void*)&pos.x);
	mKernWriteMetaball->setKernelArg( 5, sizeof(float), (void*)&pos.y);
	mKernWriteMetaball->setKernelArg( 6, sizeof(float), (void*)&pos.z);
	mCommandQueue->NDRangeKernel( mKernWriteMetaball, 1, nullptr, &count, nullptr );
}

std::vector<ci::cl::MemoryObjRef>& MarchingCubes::getAcqRelMemObjs()
{
	static std::vector<cl::MemoryObjRef> vboMem = {
		mMetaballPositions,
		mClVertPositions,
		mClVertNormals
	};
	
	return vboMem;
}

void MarchingCubes::cacheMarchingCubesMetaballData( const cl::BufferObjRef &positions, int numBalls )
{
	mMetaballPositions = positions;
	mNumBalls = numBalls;
}

void MarchingCubes::update()
{
	ivec3 size{width, height, depth};
	
	/* Update volumes */
	
	vector<cl::MemoryObjRef> acquire{
		mMetaballPositions,
		mClVertPositions,
		mClVertNormals
	};
	
	mCommandQueue->acquireGlObjects( acquire );
	
	mKernWriteMetaballs->setKernelArg( 1, sizeof(cl_int3), &size );
	mKernWriteMetaballs->setKernelArg( 2, mMetaballPositions );
	mKernWriteMetaballs->setKernelArg( 3, sizeof(cl_int), &mNumBalls );
	
	size_t work = width * height * depth;
	mCommandQueue->NDRangeKernel( mKernWriteMetaballs, 1, nullptr, &work, nullptr );
	
	/* End */
	
	int zero = 0;
	mCommandQueue->write( mClVertIndex, CL_TRUE, 0, sizeof(int), &zero);
	
	size_t num_workers = (width-1) * (height-1) * (depth-1);
	
	mKernConstructSurface->setKernelArg( 0, mClVolume );
	mKernConstructSurface->setKernelArg( 1, sizeof(cl_int3), &size );
	mKernConstructSurface->setKernelArg( 2, mClVertPositions );
	mKernConstructSurface->setKernelArg( 3, mClVertIndex );
	
	mCommandQueue->NDRangeKernel( mKernConstructSurface, 1, nullptr, &num_workers, 0 );
	
	mCommandQueue->read( mClVertIndex, CL_TRUE, 0, sizeof(cl_int), &num_verts );
	
	/* Generate Normals */
	
	if (num_verts > 0) {
		mKernGenNormalsSmooth->setKernelArg( 0, mClVertPositions );
		mKernGenNormalsSmooth->setKernelArg( 1, mClVertNormals );
		mKernGenNormalsSmooth->setKernelArg( 2, mMetaballPositions );
		mKernGenNormalsSmooth->setKernelArg( 3, sizeof(cl_int), &mNumBalls );
		mCommandQueue->NDRangeKernel( mKernGenNormalsSmooth, 1, nullptr, (size_t *)&num_verts, nullptr );
	}
	
	/*
	kernel_set_argument(generate_normals, 0, sizeof(kernel_memory), &vertex_positions_buffer);
	kernel_set_argument(generate_normals, 1, sizeof(kernel_memory), &vertex_normals_buffer);
	kernel_run(generate_normals, num_verts/3);
	*/
}

void MarchingCubes::render() {

  size_t full_size = width * height * depth;
	
	std::vector<cl::MemoryObjRef> vboMem =  { mClPointColors };
	cl::Event acquireEvent;
	mCommandQueue->acquireGlObjects( vboMem, {}, &acquireEvent );
	
	cl::EventList waitlist( { acquireEvent } );
	cl::Event kernelEvent;
	mCommandQueue->NDRangeKernel( mKernWritePointColorBack, 1, nullptr, &full_size, nullptr, waitlist, &kernelEvent );
	
	waitlist.push_back( kernelEvent );
	mCommandQueue->releaseGlObjects( vboMem, waitlist );
	
	gl::ScopedVao scopeVao( mVao );
	gl::ScopedGlslProg scopeShader( mRenderGlsl );
	
	mRenderGlsl->uniform( "light_position", vec3() );
	
	// add materials and light colors here
	// also add an environment map
	// also add shadow map and render to shadow below

	gl::setDefaultShaderVars();
	
	gl::drawArrays(GL_TRIANGLES, 0, num_verts);
}

void MarchingCubes::renderShadows() {

//  mat4 viewm = light_view_matrix(l);
//  mat4 projm = light_proj_matrix(l);
//
//  mat4_to_array(viewm, view_matrix);
//  mat4_to_array(projm, proj_matrix);
//
//  glMatrixMode(GL_MODELVIEW);
//  glLoadMatrixf(view_matrix);
//
//  glMatrixMode(GL_PROJECTION);
//  glLoadMatrixf(proj_matrix);
//
//  mat4_to_array(mat4_id(), world_matrix);
//
//  material* depth_mat = asset_get_load(P("$CORANGE/shaders/depth.mat"));
//
//  shader_program* depth_shader = material_get_entry(depth_mat, 0)->program;
//  glUseProgram(*depth_shader);
//
//  GLint world_matrix_u = glGetUniformLocation(*depth_shader, "world_matrix");
//  glUniformMatrix4fv(world_matrix_u, 1, 0, world_matrix);
//
//  GLint proj_matrix_u = glGetUniformLocation(*depth_shader, "proj_matrix");
//  glUniformMatrix4fv(proj_matrix_u, 1, 0, proj_matrix);
//
//  GLint view_matrix_u = glGetUniformLocation(*depth_shader, "view_matrix");
//  glUniformMatrix4fv(view_matrix_u, 1, 0, view_matrix);
//
//  glBindBuffer(GL_ARRAY_BUFFER, vertex_positions);
//  glVertexPointer(4, GL_FLOAT, 0, (void*)0);
//  glEnableClientState(GL_VERTEX_ARRAY);
//
//    glDrawArrays(GL_TRIANGLES, 0, num_verts);
//
//  glDisableClientState(GL_VERTEX_ARRAY);
//
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//  glUseProgram(0);
}


