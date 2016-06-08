//=============================================================================
/** @file:		scene.h
 *
 * Defines the IScene interface.
 *
	@internal
	created:	2007-10-29
	last mod:	2008-01-04

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __SCENE_H_ICNLDUDED__
#define __SCENE_H_ICNLDUDED__

#include "vector.h"

// forward declarations
class IModel;
class IShader;
class ICameraState;
class ILightingState;
class ITextureState;


//=============================================================================
//	IScene - manages test geometry and environment
//=============================================================================

/** Stores the scene and it's parameters.
 * This is the central rendering parameter container. It stores state
 * like the current test model, the camera parameters, texture iamges
 * and several state flags. Last but not least it stores the IShader object
 * that is  responsible for rendering the test model.
 */
class IScene
{
public:
	/** Creates a IScene object.
	 * The object must then be initialized with init() in order to use it.
	 */
	static IScene* create( void );
	virtual ~IScene( void ) {} ///< Destruction.

	/** Initialzes the scene object.
	 * After this call the scene is ready to operate.
	 * @pre Assumes a valid OpenGL 2.0 context active.
	 */
	virtual void init( void ) = 0;

	/** Shutdowns the object and cleans up state.
	 */
	virtual void shutdown( void ) = 0;

	/** Renders the scene with the currently set parameters.
	 */
	virtual void render( void ) = 0;


	/** Returns the ICameraState responsible for this scene.
	 */
	virtual ICameraState* getCameraState( void ) = 0;

	/** Returns the ILightingState object responsible for this scene.
	 */
	virtual ILightingState* getLightingState( void ) = 0;

	/** Returns the ITextureState object responsible for this scene.
	 */
	virtual ITextureState* getTextureState( void ) = 0;

	/** Returns the IShader object responsible for this scene
	 */
	virtual IShader* getShader( void ) = 0;


	/** Sets the viewport clear color.
	 * @param color Clear color.
	 */
	virtual void setClearColor( const vec4_t & color ) = 0;


	/** Sets the current test model.
	 * The default test model is NULL.
	 * The test model is NOT stored here, it only keeps a pointer to the model.
	 * You have to delete the model manually.
	 * @param model The test model.
	 */
	virtual void setCurrentModel( IModel* model ) = 0;

	/** Returns the current test model. */
	virtual IModel* getCurrentModel( void ) = 0;


	/** Sets the 'use program' flag.
	 * If this is set, the test model is rendered using the userdefines shader program.
	 * If this is cleared, the test model is rendered using the OpenGL fixed function pipeline.
	 * The default it true.
	 */
	virtual void setUseProgram( bool enable ) = 0;

	/** Sets the 'show origin' flag.
	 * If enabled, the world origin is shown by three lines indicationg the primary coordinate system axes.
	 * The default is false.
	 */
	virtual void setShowOrigin( bool enable ) = 0;

	/** Sets the 'show normals' flag.
	 * If this is set, the vertex normals of the current model will be drawn.
	 * The default is false.
	 */
	virtual void setShowNormals( bool enable ) = 0;

	/** Sets the 'show bounding box' flag.
	 * If this is set, the current model's bounding box will be drawn.
	 * The default is false.
	 */
	virtual void setShowBoundingBox( bool enable ) = 0;

	/** Sets the 'show tangent space vectors' flag.
	 * If this is set, the tanget space vectors of every vertex of the current model will be drawn.
	 * The default is false.
	 */
	virtual void setShowTangents( bool enable ) = 0;

	/** Sets the 'draw as wireframe' flag.
	 * If this is set, GL_POLYGON_MODE will be set to GL_LINES for the test model.
	 * Otherwise if is GL_FILL.
	 * The default is false.
	 */
	virtual void setWireframe( bool enable ) = 0;

	/** Sets the 'enable backface culling' flag.
	 * This enables/diables OpenGL back face culling for the test model.
	 * The default is false.
	 */
	virtual void setBackFaceCulling( bool enable ) = 0;
};


#endif	// __SCENE_H_ICNLDUDED__
