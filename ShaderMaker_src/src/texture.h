//=============================================================================
/** @file		texture.h
 *
 * Defines texturing management interfaces
 *
	@internal
	created:	2007-11-28
	last mod:	2008-01-28

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __TEXTURE_H_INCLUDED__
#define __TEXTURE_H_INCLUDED__

#include <QtGui/QImage>


//=============================================================================
//	ITextureState
//=============================================================================


/** This is responsible for loading and storing textures.
 * It represents an array of the texture mapping units of OpenGL.
 * Every texture mapping unit stores a 2D texture image.
 * There is only support for 2D textures. There is not power-of-two
 * limitiation, because images are scaled if necessary. There is no
 * support for customizing texture parameters like wrap-mode, filter, etc.
 */
class ITextureState
{
public:
	virtual ~ITextureState( void ) {} ///< Destruction.

	/** Returns the number of texture mapping units available.
	 * This invokes an OpenGL call and returns the value of GL_MAX_TEXTURE_UNITS.
	 * @return The implementation dependent value GL_MAX_TEXTURE_UNITS
	 */
	virtual int getMaxTextureUnits( void ) = 0;

	/** Enables/disables texture filering.
	 * This controls bilinear filtering on all textures.
	 * The initial state is false.
	 * @param enable Filtering enable state.
	 */
	virtual void setBilinearFilter( bool enable ) = 0;

	/** Assigns a texture image to a texture mapping unit.
	 * This call replaces the current image with a new image.
	 * It re-creates the complete OpenGL texture object.
	 * Depending on the image, this may take some time.
	 * Do not call this every frame, it will slow down the application.
	 * @param textureUnit Zero based index of a texture unit.
	 *        The maximum index is n-1 where n is the value
	 *        returned by glMaxTextureUnits().
	 * @param image The image to use.
	 */
	virtual void setTexture( int textureUnit, const QImage & image ) = 0;
};


#endif	// __TEXTURE_H_INCLUDED__


