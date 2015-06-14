//
//  MarchingCubes.cpp
//  MetaBalls2
//
//  Created by Ryan Bartley on 7/19/14.
//
//

#include "MarchingCubes.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Vao.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/Utilities.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace ci::gl;
using namespace cl;

::size_t full_size = width * height * depth;

int MarchingCubes::num_verts = 0;

MarchingCubesRef MarchingCubes::create( const cl::Context &context, const cl::CommandQueue &commandQueue )
{
	return MarchingCubesRef( new MarchingCubes( context, commandQueue ) );
}

MarchingCubes::MarchingCubes( const cl::Context &context, const cl::CommandQueue &commandQueue )
: mCommandQueue( commandQueue ), mDebug( false )
{
	setupGl();
	setupCl( context );
}

void MarchingCubes::setupGl()
{
	std::vector<vec4> vertPosData(MAX_VERTS);
	std::vector<vec4> vertNormData(MAX_VERTS);
	
	// Create Position Buffer
	mGlVertPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertPosData.data(), GL_DYNAMIC_COPY);
	geom::BufferLayout positionBufferLayout( { geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 ) } );
	// Create Normal Buffer
	mGlVertNormals = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * MAX_VERTS, vertNormData.data(), GL_DYNAMIC_COPY);
	geom::BufferLayout normalBufferLayout( { geom::AttribInfo( geom::NORMAL, geom::FLOAT, 4, 0, 0 ) } );
	
	{
		auto glsl = gl::GlslProg::create( gl::GlslProg::Format( )
										 .vertex( loadAsset( "phong.vert" ) )
										 .fragment( loadAsset( "phong.frag" ) ) );
		auto vboMesh = gl::VboMesh::create( MAX_VERTS, GL_TRIANGLES, {
			{ positionBufferLayout, mGlVertPositions },
			{ normalBufferLayout, mGlVertNormals }
		} );
		mRenderBatch = gl::Batch::create( vboMesh, glsl );
//		mRenderGlsl = gl::GlslProg::create( gl::GlslProg::Format() );
//		mShadowGlsl = gl::GlslProg::create( gl::GlslProg::Format() );
//		
//		mRenderBatch = gl::Batch::create( vboMesh, mRenderGlsl );
//		mShadowBatch = gl::Batch::create( vboMesh, mShadowGlsl );
	}
	
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
	
	// Create Position Indices Buffer
	mGlPointPositions = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointData.data(), GL_STATIC_DRAW);
	geom::BufferLayout positionIndexBufferLayout( { geom::AttribInfo( geom::POSITION, geom::FLOAT, 4, 0, 0 ) } );
	// Create Color Buffer
	mGlPointColors = gl::Vbo::create(GL_ARRAY_BUFFER, sizeof(vec4) * full_size, pointColorData.data(), GL_DYNAMIC_COPY);
	geom::BufferLayout colorBufferLayout( { geom::AttribInfo( geom::COLOR, geom::FLOAT, 4, 0, 0 ) } );
	
	{
		auto glsl = gl::GlslProg::create( gl::GlslProg::Format( )
										 .vertex( loadAsset( "basic.vert" ) )
										 .fragment( loadAsset( "basic.frag" ) ) );
		
		auto vboMesh = gl::VboMesh::create( full_size, GL_POINTS, {
			{ positionIndexBufferLayout, mGlPointPositions },
			{ colorBufferLayout, mGlPointColors },
		} );
		mDebugBatch = gl::Batch::create( vboMesh, glsl );
	}
	mEnvironmentMap = gl::Texture2d::createFromDds( loadAsset( "metaballs_env.dds" ) );
}

void MarchingCubes::setupCl( const cl::Context &context )
{
	mClPointColors   =	cl::BufferGL( context, CL_MEM_READ_WRITE, mGlPointColors->getId() );
	mClVertPositions =	cl::BufferGL( context, CL_MEM_READ_WRITE, mGlVertPositions->getId() );
	mClVertNormals   =	cl::BufferGL( context, CL_MEM_READ_WRITE, mGlVertNormals->getId() );
	mClVolume		 =	cl::Buffer( context, CL_MEM_READ_WRITE, sizeof(float) * full_size, nullptr );
	mClVertIndex     =	cl::Buffer( context, CL_MEM_READ_WRITE, sizeof(int), nullptr );
	
	
	mClProgram = cl::Program( context, loadString( loadAsset( "kernels/marching_cubes.cl" ) ) );
	auto path = "-I" + getAssetPath( "" ).string();
	mClProgram.build( path.c_str() );
	
	mKernWritePoint = cl::Kernel( mClProgram, "write_point" );
	mKernWritePoint.setArg( 0, mClVolume );
	mKernWritePoint.setArg( 4, sizeof(int), (void*)&width );
	mKernWritePoint.setArg( 5, sizeof(int), (void*)&height );
	mKernWritePoint.setArg( 6, sizeof(int), (void*)&depth );
	
	mKernWriteMetaball = cl::Kernel( mClProgram, "write_metaball" );
	mKernWriteMetaball.setArg( 0, mClVolume );
	
	mKernWriteMetaballs = cl::Kernel( mClProgram, "write_metaballs" );
	mKernWriteMetaballs.setArg( 0, mClVolume );
	
	mKernWriteClear = cl::Kernel( mClProgram, "write_clear" );
	mKernWriteClear.setArg( 0, mClVolume );
	
	mKernWritePointColorBack = cl::Kernel( mClProgram, "write_point_color_back" );
	mKernWritePointColorBack.setArg( 0, mClVolume );
	mKernWritePointColorBack.setArg( 1, mClPointColors );
	
	mKernConstructSurface = cl::Kernel( mClProgram, "construct_surface" );
	mKernConstructSurface.setArg( 0, mClVolume );
	
	mKernGenNormals = cl::Kernel( mClProgram, "generate_flat_normals" );
	mKernGenNormalsSmooth = cl::Kernel( mClProgram, "generate_smooth_normals" );
}

void MarchingCubes::clear()
{
	mCommandQueue.enqueueNDRangeKernel( mKernWriteClear,
									   cl::NullRange,
									   cl::NDRange( full_size ),
									   cl::NullRange );
}

void MarchingCubes::point( const ivec4 &point ) {

	if ((point.x >= width) || (point.y >= height) || (point.z >= depth) || (point.x < 0) || (point.y < 0) || (point.z < 0)) {
		CI_LOG_E("Point outside of volume");
	}

	mKernWritePoint.setArg( 1, sizeof(int), (void*)&point.x );
	mKernWritePoint.setArg( 2, sizeof(int), (void*)&point.y );
	mKernWritePoint.setArg( 3, sizeof(int), (void*)&point.z );
	mKernWritePoint.setArg( 7, sizeof(float), (void*)&point.w );
	
	mCommandQueue.enqueueNDRangeKernel( mKernWritePoint,
									   cl::NullRange,
									   cl::NDRange( 1 ),
									   cl::NullRange );
}

void MarchingCubes::metaball( const ci::vec3 &pos ) {
	const int METABALL_SIZE = 10;

	int bot_x = math<int>::max(floor(pos.x) - METABALL_SIZE, 0);
	int bot_y = math<int>::max(floor(pos.y) - METABALL_SIZE, 0);
	int bot_z = math<int>::max(floor(pos.z) - METABALL_SIZE, 0);

	int top_x = math<int>::min(ceil(pos.x) + METABALL_SIZE, width-1);
	int top_y = math<int>::min(ceil(pos.y) + METABALL_SIZE, height-1);
	int top_z = math<int>::min(ceil(pos.z) + METABALL_SIZE, depth-1);

	::size_t count = (top_x - bot_x) * (top_y - bot_y) * (top_z - bot_z);

	ivec3 bottom{bot_x, bot_y, bot_z};
	ivec3 top	{top_x, top_y, top_z};
	ivec3 size	{width, height, depth};

	mKernWriteMetaball.setArg( 1, sizeof(ivec3), &bottom );
	mKernWriteMetaball.setArg( 2, sizeof(ivec3), &top);
	mKernWriteMetaball.setArg( 3, sizeof(ivec3), &size);
	mKernWriteMetaball.setArg( 4, sizeof(float), (void*)&pos.x);
	mKernWriteMetaball.setArg( 5, sizeof(float), (void*)&pos.y);
	mKernWriteMetaball.setArg( 6, sizeof(float), (void*)&pos.z);
	mCommandQueue.enqueueNDRangeKernel( mKernWriteClear,
									   cl::NullRange,
									   cl::NDRange( count ),
									   cl::NullRange );
}

void MarchingCubes::cacheMarchingCubesMetaballData( const cl::BufferGL &positions, int numBalls )
{
	mMetaballPositions = positions;
	mNumBalls = numBalls;
}

std::vector<cl::Memory> MarchingCubes::getInterop()
{
	vector<cl::Memory> ret{
		mClVertPositions,
		mClVertNormals,
		mClPointColors
	};
	return ret;
}

std::vector<cl::Event> MarchingCubes::update()
{
	ivec3 size{width, height, depth};
	std::vector<cl::Event> waitEvents;
	/* Update volumes */
	clear();
	mKernWriteMetaballs.setArg( 1, sizeof(cl_int3), &size );
	mKernWriteMetaballs.setArg( 2, mMetaballPositions );
	mKernWriteMetaballs.setArg( 3, sizeof(cl_int), &mNumBalls );
	
	mCommandQueue.enqueueNDRangeKernel( mKernWriteMetaballs,
									   cl::NullRange,
									   cl::NDRange(  width * height * depth ),
									   cl::NullRange );
	
	/* End */
	
	int zero = 0;
	cl::Event writeEvent;
	mCommandQueue.enqueueWriteBuffer( mClVertIndex, false, 0, sizeof(int), &zero, nullptr, &writeEvent );
	
	mKernConstructSurface.setArg( 0, mClVolume );
	mKernConstructSurface.setArg( 1, sizeof(cl_int3), &size );
	mKernConstructSurface.setArg( 2, mClVertPositions );
	mKernConstructSurface.setArg( 3, mClVertIndex );
	
	waitEvents.push_back( writeEvent );
	
	cl::Event constructEvent;
	mCommandQueue.enqueueNDRangeKernel( mKernConstructSurface,
										cl::NullRange,
										cl::NDRange( (width-1) * (height-1) * (depth-1) ),
										cl::NullRange,
										&waitEvents,
										&constructEvent );
	waitEvents.push_back( constructEvent );
	cl::Event readEvent;
	mCommandQueue.enqueueReadBuffer( mClVertIndex, true, 0, sizeof(cl_int), &num_verts, &waitEvents, &readEvent );
	waitEvents.push_back( readEvent );
	
	/* Generate Normals */
	if (num_verts > 0) {
		bool smooth = true;
		cl::Event normals;
		if( ! smooth ) {
			mKernGenNormals.setArg( 0, mClVertPositions );
			mKernGenNormals.setArg( 1, mClVertNormals );
			mCommandQueue.enqueueNDRangeKernel( mKernGenNormals,
												cl::NullRange,
												cl::NDRange( num_verts ),
												cl::NullRange,
												&waitEvents,
												&normals );
		}
		else {
			mKernGenNormalsSmooth.setArg( 0, mClVertPositions );
			mKernGenNormalsSmooth.setArg( 1, mClVertNormals );
			mKernGenNormalsSmooth.setArg( 2, mMetaballPositions );
			mKernGenNormalsSmooth.setArg( 3, sizeof(cl_int), &mNumBalls );
			mCommandQueue.enqueueNDRangeKernel( mKernGenNormalsSmooth,
												cl::NullRange,
												cl::NDRange( num_verts ),
												cl::NullRange,
												&waitEvents,
												&normals );
		}
		waitEvents.push_back( normals );
	}

	if( mDebug ) {
		cl::Event pointColor;
		mCommandQueue.enqueueNDRangeKernel( mKernWritePointColorBack,
											cl::NullRange,
											cl::NDRange( full_size ),
											cl::NullRange,
											&waitEvents,
											&pointColor );
		waitEvents.push_back( pointColor );
	}
	return waitEvents;
}

void MarchingCubes::render()
{
	renderShadows();
//	mDebugBatch->draw();
	mRenderBatch->draw( 0, num_verts );
//	gl::ScopedVao scopeVao( mVao );
//	gl::ScopedGlslProg scopeShader( mRenderGlsl );
	
//	mRenderGlsl->uniform( "light_position",  );
	
	// add materials and light colors here
	// also add an environment map
	// also add shadow map and render to shadow below
	
	
//	GLuint NORMALS = glGetAttribLocation(*metaballs, "normals");
//	
//	glUseProgram(*metaballs);
//	
//	GLint light_position_u = glGetUniformLocation(*metaballs, "light_position");
//	glUniform3f(light_position_u, l->position.x, l->position.y, l->position.z);
//	
//	GLint camera_position_u = glGetUniformLocation(*metaballs, "camera_position");
//	glUniform3f(camera_position_u, c->position.x, c->position.y, c->position.z);
//	
//	mat4 lviewm = light_view_matrix(l);
//	mat4 lprojm = light_proj_matrix(l);
//	
//	mat4_to_array(lviewm, lview_matrix);
//	mat4_to_array(lprojm, lproj_matrix);
//	
//	GLint lproj_matrix_u = glGetUniformLocation(*metaballs, "light_proj");
//	glUniformMatrix4fv(lproj_matrix_u, 1, 0, lproj_matrix);
//	
//	GLint lview_matrix_u = glGetUniformLocation(*metaballs, "light_view");
//	glUniformMatrix4fv(lview_matrix_u, 1, 0, lview_matrix);
//	
//	texture* env_map = asset_get_load(P("./resources/metaballs_env.dds"));
//	glActiveTexture(GL_TEXTURE0 + 0 );
//	glBindTexture(GL_TEXTURE_2D, texture_handle(env_map));
//	glEnable(GL_TEXTURE_2D);
//	glUniform1i(glGetUniformLocation(*metaballs, "env_map"), 0);
//	
//	glBindBuffer(GL_ARRAY_BUFFER, vertex_positions);
//	glVertexPointer(4, GL_FLOAT, 0, (void*)0);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	
//	glBindBuffer(GL_ARRAY_BUFFER, vertex_normals);
//	glVertexAttribPointer(NORMALS, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
//	glEnableVertexAttribArray(NORMALS);
//	
//	  glDrawArrays(GL_TRIANGLES, 0, num_verts);
//	
//	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableVertexAttribArray(NORMALS);
//	
//	glActiveTexture(GL_TEXTURE0 + 1 );
//	glDisable(GL_TEXTURE_2D);
//	
//	glActiveTexture(GL_TEXTURE0 + 0 );
//	glDisable(GL_TEXTURE_2D);
//	
//	glUseProgram(0);
//	
//	if (wireframe) {
//	  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
//	}
//	
//	gl::setDefaultShaderVars();
//	
//	gl::drawArrays(GL_TRIANGLES, 0, num_verts);
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


