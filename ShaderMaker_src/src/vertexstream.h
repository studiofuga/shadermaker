//=============================================================================
/** @file		vertexstream.h
 *
 * Defines a vertex buffer interface.
 *
	@internal
	created:	2007-12-14
	last mod:	2008-01-07

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __VERTEXSTREAM_H_INCLUDED__
#define __VERTEXSTREAM_H_INCLUDED__

#include "vector.h"

// forward declarations
class VertexAttribLocations;


//=============================================================================
//	IVertexStream
//=============================================================================

/** An interface to a generic vertex data container.
 * The container is an easy-to-use wrapper for OpenGL vertex arrays.
 * It supports some custom vertex attributes for vertex shaders.
 */
class IVertexStream
{
public:
	/** creates an IVertexStream object.
	 * @param numVertices Number of vertices available in the buffer.
	 */
	static IVertexStream* create( int numVertices );
	virtual ~IVertexStream( void ) {} ///< Destructor, provided for compatibility.

	/** Computes tangent space vectors 'tangent' and 'bitangent'.
	 * @warning Assumes individual triangles are stored in the stream.
	 * Calculates for each triangle in the buffer the vertex attributes
	 * 'attrTangent' and 'attrBitangent'. These vectors are based on the
	 * vertex normals, texture coords and positions.
	 */
	virtual void coumputeTangentVectors( void ) = 0;

	/** Computes the bounding radius.
	 * It simply returns the maximum length of all vertex positions
	 * stored in the stream.
	 * @return The bounding radius
	 */
	virtual float computeBoundingRadius( void ) = 0;

	/** Sends the stream to OpenGL.
	 * It setups OpenGL client state, binds vertex arrays, draws the
	 * complete array and cleans up the GL client state.
	 * @param primitiveType The primitive type that is passed to OpenGL.
	 * @param overrideColor If != NULL, this color will be passed to
	 *        OpenGL instead of the colors stored in the stream.
	 * @param attribs If != NULL the custom vertex attributes are send to
	 *                the locations defined in the parameter.
	 */
	virtual void render( int primitiveType, const vec4_t * overrideColor = NULL,
						 const VertexAttribLocations * attribs = NULL ) = 0;

	/** Draws the normals of all vertices.
	 * It loops through all vertices and draws a colored line starting
	 * at the vertex position and pointing into the normal's direction.
	 * The colors are chosen from the largest component:
	 *  x == red, Y == green, Z == blue.
	 */
	virtual void renderNormals( void ) = 0;

	/** Draw the basis vectors of the tangent space for all vertices.
	 * It draws a line from the vertex position in the direction of 
	 * each tangent space vector.
	 * Tangent == red, Bitangent == green, Normal == blue.
	 */
	virtual void renderTangentVectors( void ) = 0;

	// vertex arrays access
	virtual vec3_t*	v( void ) = 0; ///< Returns the vertex position array.
	virtual vec3_t*	n( void ) = 0; ///< Returns the normal array.
	virtual vec2_t*	t( void ) = 0; ///< Returns the tex coord array.
	virtual vec4_t*	c( void ) = 0; ///< Returns the primary color array.
	virtual vec3_t* tan1( void ) = 0; ///< Returns the tangent array.
	virtual vec3_t* tan2( void ) = 0; ///< Returns the bitangent array.
};


#endif	// __VERTEXSTREAM_H_INCLUDED__


