//=============================================================================
/** @file		camera.h
 *
 * Defines the camera interface.
 *
	@internal
	created:	2007-11-30
	last mod:	2008-02-26

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __CAMERA_H_INCLUDED__
#define __CAMERA_H_INCLUDED__

#include "vector.h"


//=============================================================================
//	ICameraState
//=============================================================================

/** Stores camera parameters.
 * This interface is used to manipulate camera related parameters
 * like projection mode and transformation.
 */
class ICameraState
{
public:
	/** Destructor. */
	virtual ~ICameraState( void ) {}

	/** Defines camera projection modes. */
	enum projMode_e { PROJECT_ORTHO, PROJECT_FRUSTUM, };


	/** Reset camera to initial position and angles.
	 * Does not change the projection mode
	 */
	virtual void resetCamera( void ) = 0;


	/** Sets the camera rotation matrix.
	 * The matrix is 4x4 and all matrix elements are used.
	 * The matrix is strored in column major order.
	 * @param matrix The rotation matrix
	 * @pre Assumes the matrix is a rotation matrix.
	 *      If this is not the case, results are undefined.
	 */
	virtual void setCameraRotation( const mat4_t & matrix ) = 0;


	/** Get the camera rotation matrix.
	 * The matrix is 4x4 and all elements are used.
	 * @param matrix The rotation maxtrix buffer.
	 */
	virtual void getCameraRotation( mat4_t & matrix ) = 0;


	/** Sets the new camera translation vector.
	 * This translation is applied after the camera was rotated.
	 * @param t Translation vector.
	 */
	virtual void setCameraTranslation( const vec3_t & t ) = 0;


	/** Get the current camera translation vector.
	 * @param t The translation vector will be stored there.
	 */
	virtual void getCameraTranslation( vec3_t & t ) = 0;


	/** Set the projection mode.
	 * The default is PROJECT_FRUSTUM.
	 * @param mode Projection mode.
	 */
	virtual void setProjectionMode( projMode_e mode ) = 0;


	/** Set the vertical field-of-view in degree.
	 * The value can range from >0 degree up to < 180 degree.
	 * If the value is outside this range, it is set to 50.
	 * The horizontal fov depends on the width/height aspect of the viewport,
	 * where fovX = ( width / height ) * fovY.
	 * The default value is 50.
	 * @param fovY Vertical field-of-view in degree.
	 */
	virtual void setFovY( float fovY ) = 0;
};

#endif	// __CAMERA_H_INCLUDED__

