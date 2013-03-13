//=============================================================================
/** @file		stdshader.h
 *
 * Defines the shaders initially set during application startup
 *
	@internal
	created:	2007-11-21
	last mod:	2008-01-12

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __STDSHADER_H_INCLUDED__
#define __STDSHADER_H_INCLUDED__


//=============================================================================
//	initial vertex shader
//=============================================================================

/** Initial vertex shader code.
 */
#define INITIAL_VERTEX_SHADER_SOURCE \
	"// simple vertex shader\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"\tgl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;\n" \
	"\tgl_FrontColor  = gl_Color;\n" \
	"\tgl_TexCoord[0] = gl_MultiTexCoord0;\n" \
	"}\n"


//=============================================================================
//	initial geometry shader
//=============================================================================

/** Initial geometry shadre source
 */
#define INITIAL_GEOMETRY_SHADER_SOURCE \
	"// simple geometry shader\n" \
	"\n" \
	"// these lines enable the geometry shader support.\n" \
	"#version 120\n" \
	"#extension GL_EXT_geometry_shader4 : enable\n" \
	"\n" \
	"void main( void )\n" \
	"{\n" \
	"\tfor( int i = 0 ; i < gl_VerticesIn ; i++ )\n" \
	"\t{\n" \
	"\t\tgl_FrontColor  = gl_FrontColorIn[ i ];\n" \
	"\t\tgl_Position    = gl_PositionIn  [ i ];\n" \
	"\t\tgl_TexCoord[0] = gl_TexCoordIn  [ i ][ 0 ];\n" \
	"\t\tEmitVertex();\n" \
	"\t}\n" \
	"}\n"


//=============================================================================
//	initial fragment shader
//=============================================================================

/** Initial fragment shader code.
 */
#define INITIAL_FRAGMENT_SHADER_SOURCE \
	"// simple fragment shader\n" \
	"\n" \
	"// 'time' contains seconds since the program was linked.\n" \
	"uniform float time;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	gl_FragColor = gl_Color;\n" \
	"}\n"



#endif	// __STDSHADER_H_INCLUDED__
