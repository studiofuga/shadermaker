//=============================================================================
/** @file		light.h
 *
 * Defines lighting state related classes.
 *
	@internal
	created:	2007-11-06
	last mod:	2008-01-05

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __LIGHT_H_INCLUDED__
#define __LIGHT_H_INCLUDED__

#include "vector.h"


//=============================================================================
//	CLight
//=============================================================================

/** A collection of OpenGL light source parameters.
 * This class allows storing the most important OpenGL light source
 * parameters like position, colors and the enabled bit.
 * It also supports some application specific state, like a flag wether
 * the light is locked to a camera or not. In the first case, the light
 * position is specified in eye coordinates, in the other case world coordinats.
 * Another feature is a flag, wether a light should be auto-rotated around
 * the origin. This takes only effect, if the light is not locked to the
 * camera position. Auto-rotation can be used to test the behavoir of lighting
 * shaders without manipulating many light positions by hand. The exact auto-rotate
 * transformation is implementation specific.
 * \n\n
 * The methods of this class are only used for reading/writing light source
 * parameters and have no side effects. That's why they are less documented.
 */
class CLight
{
public:
	/** Constructs a CLight object.
	 * It defaults all colors to back (0,0,0,0) and
	 * all state bits (enabled, locked-to-camera, auto-rotate) to False.
	 */
	CLight( void )
	{
		m_enabled = false;
		m_lockedToCamera = false;
		m_autoRotate = false;
	}

	// enabled
	bool	getEnabled( void ) const { return m_enabled; } ///< Reads the enabled bit.
	void	setEnabled( bool en ) { m_enabled = en; } ///< Writes enabled state bit.

	// lock to camera or world
	bool	getLockedToCamera( void ) const { return m_lockedToCamera; } ///< Reads lock-to-camera state bit.
	void	setLockedToCamera( bool locked ) { m_lockedToCamera = locked; } ///< Writes lock-to-camera state bit.

	// auto-rotate
	bool	getAutoRotate( void ) const { return m_autoRotate; } ///< Reads auto-rotate state bit.
	void	setAutoRotate( bool enable ) { m_autoRotate = enable; } ///< Writes auto-rotate state bit.

	// position
	vec4_t	getPosition( void ) const { return m_position; } ///< Reads light position.
	void	setPosition( const vec4_t & v ) { m_position = v; } ///< Writes light position.

	// ambient
	vec4_t	getAmbient ( void ) const { return m_ambient; } ///< Reads ambient color.
	void	setAmbient ( const vec4_t & v ) { m_ambient = v; } ///< Writes ambient color.

	// diffuse
	vec4_t	getDiffuse ( void ) const { return m_diffuse; } ///< Reads diffuse color.
	void	setDiffuse ( const vec4_t & v ) { m_diffuse = v; } ///< Writes diffuse color.

	// specular
	vec4_t	getSpecular( void ) const { return m_specular; } ///< Reads specular color.
	void	setSpecular( const vec4_t & v ) { m_specular = v; } ///< Writes specular color.

private:
	bool		m_enabled;
	bool		m_lockedToCamera;
	bool		m_autoRotate;
	vec4_t		m_position;
	vec4_t		m_ambient;
	vec4_t		m_diffuse;
	vec4_t		m_specular;
};


//=============================================================================
//	CMaterial - stores material parameters
//=============================================================================

/** A collection of OpenGL material parameters.
 * This class allows storing the most important OpenGL material
 * parameters like colors and the specular exponent.
 * It also supports a flag, wether the diffuse color should be taken form
 * the current vertex color instead of the diffuse color stored.
 * The initial state of the lighting parameters is the same like that
 * defined in the OpenGL specification.
 * \n\n
 * The methods of this class are only used for reading/writing material
 * parameters and have no side effects. That's why they are less documented.
 */
class CMaterial
{
public:
	/** Constructs a CMaterial object.
	 * It initializes all colors to back (0,0,0,0) and the specular
	 * exponent to zero. The use-vertex-color flag is set to False.
	 */
	CMaterial( void )
	{
		m_useVertexColor = false;
		m_specularExponent = 0.0f;
	}

	// emission
	vec4_t	getEmission( void ) const { return m_emission; } ///< Reads emission color.
	void	setEmission( const vec4_t & v ) { m_emission = v; } ///< Writes emission color.

	// ambient
	vec4_t	getAmbient( void ) const { return m_ambient; } ///< Reads ambient color.
	void	setAmbient( const vec4_t & v ) { m_ambient = v; } ///< Writes ambient color.

	// diffuse
	vec4_t	getDiffuse( void ) const { return m_diffuse; } ///< Reads diffuse color.
	void	setDiffuse( const vec4_t & v ) { m_diffuse = v; } ///< Writes diffuse colors.

	// specular
	vec4_t	getSpecular( void ) const { return m_specular; } ///< Reads specular color.
	void	setSpecular( const vec4_t & v ) { m_specular = v; } ///< Writes specular color.

	// specular exponent
	float	getSpecularExponent( void ) const { return m_specularExponent; } ///< Reads the specular exponent.
	void	setSpecularExponent( float f ) { m_specularExponent = f; } ///< Writes the specular exponent.

	// color material flag
	bool	getUseVertexColor( void ) const { return m_useVertexColor; } ///< Reads the use-vertex-color flag.
	void	setUseVertexColor( bool enable ) { m_useVertexColor = enable; } ///< Writes the use-vertex-color flag.

private:
	vec4_t	m_emission;
	vec4_t	m_ambient;
	vec4_t	m_diffuse;
	vec4_t	m_specular;
	float	m_specularExponent;
	bool	m_useVertexColor; // as diffuse
};


//=============================================================================
//	ILightingState - stores lighting state
//=============================================================================

/** Stores lighting parameters.
 * This interface is responsible for storing lighting specific state,
 * like the lighting-enabled flag, light sources and the current material.
 * It basically represents what is defined in the OpenGL specification.
 * \n\n
 * The show-lights flag defines, wether the light sources should be drawn
 * as spheres in the diffuse color of each light source.
 * \n\n
 * The maximum amount of light sources available can be read from the
 * symbolic constant MAX_LIGHTS defined in this interface.
 * \n\n
 * This interface implicitly controls how lights and material are processed.
 * See CLight for additional documentation about light state processing.
 */
class ILightingState
{
public:
	/** Destructor. */
	virtual ~ILightingState( void ) {}

	/** Symbolic constant for maximum number of
	 * lights available in a ILightingState object.
	 * The value 8 is the minimum value allowed
	 * for the implementation specific constant GL_MAX_LIGHTS.
	 */
	enum { MAX_LIGHTS = 8 };

	/** Writes the show-lights flag.
	 * @param enable The show-lights flag.
	 */
	virtual void setShowLights( bool enable ) = 0;

	/** Sets the light source parameters of an indexed light.
	 * @param index Light source index, must be in range [0,MAX_LIGHTS-1].
	 * @param l Light source parameters.
	 */
	virtual void setLight( int index, const CLight & l ) = 0;

	/** Sets the current material parameters.
	 * @param m Material parameters.
	 */
	virtual void setMaterial( const CMaterial & m ) = 0;

	/** Writes lighting-enables state bit.
	 * This represents the GL_LIGHTING state bit of OpenGL.
	 * The initial state is false.
	 * @param enable The lighting-enabled state bit.
	 */
	virtual void setLightingEnabled( bool enable ) = 0;

	/** Get parameters of an indexed light source.
	 * The call fails, if the light source index is out of range.
	 * In that case, results are undefined.
	 * @param index Light source index, must be in range [0,MAX_LIGHTS-1].
	 * @param l Buffer that is filled with ligh source parameters.
	 * @return Wether the call succeeded.
	 */
	virtual bool getLight( int index, CLight & l ) const = 0;

	/** Get the current material parameters.
	 * @param m Buffer that is filled with material parameters.
	 */
	virtual void getMaterial( CMaterial & m ) const = 0;

	/** Reads the lighting-enabled state bit.
	 * @return The lighting-enabled state bit.
	 */
	virtual bool getLightingEnabled( void ) const = 0;
};


#endif	// __LIGHT_H_INCLUDED__
