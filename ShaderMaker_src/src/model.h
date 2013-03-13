//=============================================================================
/** @file		model.h
 *
 * Defines the a geometry buffer (model) interface.
 * 
	@internal
	created:	2007-12-18
	last mod:	2008-01-25

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __MODEL_H_INCLUDED__
#define __MODEL_H_INCLUDED__

#include <QtCore/QString>
#include "vector.h"

// forward declarations
class VertexAttribLocations;


//=============================================================================
//	IModel - stores geometry for rendering.
//=============================================================================

/** A geometry buffer.
 * This is an interface to a buffer, that stores geometry ready for rendering.
 * This is basically a container for IVertexStream meta data.
 * It stores a IVertexStream object and some extra infos like bounding box,
 * bounding radius, primitive type and model name.
 * It provides several factory methods for object creation.
 */
class IModel
{
public:
	// factory
	static IModel* createPoint( void ); // a single point located in the origin.
	static IModel* createPlane( void );
	static IModel* createCube ( void );
	static IModel* createSphere( int numRings, int numSegments, float radius );
	static IModel* createTorus ( int numRings, int numSegments, float radius1, float radius2  );
	virtual ~IModel( void ) {} ///< Destructor.

	/** Returns the name of this model. */
	virtual QString getName( void ) = 0;

	/** Sends the stored geometry to OpenGL via vertex arrays.
	 * It enables OpenGL client state, binds vertex arrays, draws them
	 * and disables the client state.
	 *
	 * @param attribs Custom vertex attribute locations. If this model
	 *			has these attributes available, they will be bound
	 *			to the indexed custom vertex attributes.
	 *			If one of these attributes is -1, it will not be used.
	 *			If this parameters is set to NULL, no custom attributes will be used.
	 * @param overrideColor If this parameters if != NULL, then this color
	 *			will be used as vertex color instead of the values stored
	 *			in this model.
	 */
	virtual void render( const VertexAttribLocations * attribs = NULL,
						 const vec4_t * overrideColor = NULL ) = 0;

	/** Draws the vertex normals stored in this model.
	 * If no normal are available, this call has no effect.
	 * It loops through all vertices and draws a colored line starting
	 * at the vertex position and pointing into the normal's direction.
	 * The colors are chosen from the largest component:
	 *  x == red, Y == green, Z == blue.
	 */
	virtual void renderNormals( void ) = 0;

	/** Draw vertex tangent space basis stored in this model.
	 * If no tangent space vectors are available,
	 * this call has no effect. Otherwise it draws a line from the vertex position
	 * in the direction of each tangent space vector.
	 * Tangent == red, Bitangent == green, Normal == blue.
	 */
	virtual void renderTangents( void ) = 0;

	/** Returns the primitive type of this model.
	 * It assumes that the model is constructed of only one primitive type.
	 * @return OpenGL primitive type of this model.
	 */
	virtual int getPrimitiveType( void ) = 0;

	/** Returns the primitive type if this model in a human readable format.
	 * It assumes that the model is constructeed of only one primitive type.
	 * @return OpenGL primitive type of this model as human readable string.
	 */
	virtual QString getPrimitiveTypeName( void ) = 0;

	/** Returns the bounding radius of this model.
	 * @return A bounding sphere radius that can be used for culling, etc.
	 */
	virtual float getBoundingRadius( void ) = 0;

	/** Returns the bounding box of this model.
	 * The bounding box is defined by minimum and maximum coordinates.
	 * @param mins Buffer to store the minimum coordiantes of the bounding box.
	 * @param maxs Buffer to store the maximum coordiantes of the bounding box.
	 */
	virtual void  getBoundingBox( vec3_t & mins, vec3_t & maxs ) = 0;

	/** Maps OpenGL symbolic constants into strings.
	 * @param primitiveType OpenGL primitive type.
	 *			Valid types are defined in the OpenGL 2.0 specification.
	 * @return String representation of the input primitive type.
	 */
	static QString primitiveTypeName( int primitiveType );
};


//=============================================================================
//	IMeshModel
//=============================================================================

/** An IModel dedicated for loading models from files.
 * This class can be used to access models stored in files.
 * After construction, no model data is available. It must be loaded with loadObjModel().
 * Currently only a subset of Wavefront .OBJ models files are supported.
 * It can load the model's geometry, including position, normals and texture coords.
 * It scales the model to fit into a unit shere. Missing attributes are
 * filled with default values. \n
 * \n
 * \n
 * This class does not support loading of external material files, because
 * IModel does not support material properties. Materials must be set by the user
 * in the material dialog.
 */
class IMeshModel : public IModel
{
public:
	static IMeshModel* createMeshModel( void );

	/** Loads a model from a file.
	 * The file to load is assumed to be of .OBJ format.
	 * If loading fails, then this object looses the data stored in it.
	 * @param fileName Name of the file to load.
	 * @return Ture if loading succeeded, flase otherwise.
	 */
	virtual bool loadObjModel( const QString & fileName ) = 0;
};


#endif	// __MODEL_H_INCLUDED__

