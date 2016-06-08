//=============================================================================
/** @file		shader.h
 *
 * Defines the IShader interface
 *
	@internal
	created:	2007-10-29
	last mod:	2008-01-12

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __SHADER_H_ICNLDUDED__
#define __SHADER_H_ICNLDUDED__

#include <QtCore/QString>

#include "uniform.h"

// forward declarations
class VertexAttribLocations;


//=============================================================================
//	IShader - collects user defined shader state
//=============================================================================

/** Represents a GLSL program object.
 * Provides methods for compiling and linking GLSL source code
 * to a program object. It supports vertex, fragment and geometry shaders.
 * This interface requires a valid OpenGL 2.0 context to operate.
 * If this object is initialized without such a context, behavior is undefined.
 */
class IShader : public IUniformState
{
public:
	/** Factory for IShader objects.
	 * IShader objects can be created without a valid OpenGL context in use.
	 * @return A new IShader instance.
	 */
	static IShader* create( void );
	virtual ~IShader( void ) {} ///< Destructor


	/** Initialized the object.
	 * After this call the object is ready to compile and link GLSL code.
	 * @pre Requeires a valid OpenGL context in use.
	 */
	virtual bool init( void ) = 0;


	/** Destroys all OpenGL objects and goes back to uninitialized state.
	 */
	virtual void shutdown( void ) = 0;


	/** Setups OpenGL to use that program.
	 * If the program was not suceessfully linked, then this call
	 * makes OpenGL use the fixed-function pipeline.
	 * @param attribs Locations of used custom vertex attributes will
	 *			be returned in this argument. If a vertex attribute
	 *			is not available, that attribute will be set to -1.
	 * @return True, if the program is in use, False otherwise.
	 */
	virtual bool bindState( VertexAttribLocations & attribs ) = 0;


	/** Compiles shaders and links the program.
	 * Is also setups uniform lists and build log.
	 * If no sources are specified for all shaders, this call
	 * will return true although no program is generated.
	 * @return True if the program was successfully linked. False otherwise.
	 */
	virtual bool compileAndLink( void ) = 0;


	/** Destroys the current program and makes it unuseable.
	 * Future calls to bindState() will fail until a call to
	 * compileAndLink() successfully created a program.
	 */
	virtual void deactivateProgram( void ) = 0;


	/** Returns a string with compile and link results.
	 * This log also includes information about active uniforms and
	 * vertex attributes. The log is already formatted, so it can
	 * be displayed to the user.
	 * @return The build log string.
	 */
	virtual QString getBuildLog( void ) = 0;


	/** Assigns GLSL source code to a shader.
	 * Old source code is replaced with the new code.
	 * the program is not changed until compileAndLink() is called.
	 * An empty string indicates that the specified shader type should
	 * not be attached to the program.
	 * @param shaderType The shader type to replace.
	 * @param source The source code.
	 */
	virtual void setShaderSource( int shaderType, const QString & source ) = 0;


	/** Sets the geometry shaders input primitive type.
	 * If the program does not support geometry shaders, this call has no effect.
	 * If the primitive type is invalid, this call has no effect.
	 * This call does not change the porgam. You will have to relink the program
	 * for the changes to take effect.
	 * @param type OpenGL primitive type. Valid types are those specified in the
	 *				GL_EXT_geometry_shader4 extension.
	 */
	virtual void setGeometryInputType( int type ) = 0; // geometry shader only


	/** Sets the geometry shaders output primitive type.
	 * If the program does not support geometry shaders, this call has no effect.
	 * If the primitive type is invalid, this call has no effect.
	 * This call does not change the porgam. You will have to relink the program
	 * for the changes to take effect.
	 * @param type OpenGL primitive type. Valid types are those specified in the
	 *				GL_EXT_geometry_shader4 extension.
	 */
	virtual void setGeometryOutputType( int type ) = 0;
    virtual void setGeometryOutputNum( int type ) = 0;


	/** Check wether a given shader type is avilable for this program.
	 * Returns false for invalid input.
	 * @param type Shader type identifier, defined in shaderType_e
	 * @return Wether the shader type is supported by the program.
	 */
	virtual bool isShaderTypeAvailable( int type ) = 0;

	/** Translates values from shaderType_e into human readable strings.
	 * Returns an error string for invalid input.
	 * @param type Shader type to translate.
	 * @return A human readable shader type name.
	 */
	static QString getShaderTypeName( int type );

	/** OpenGL independend shader types.
	 * They can be used as indices into arrays in a for loop.
	 */
	enum shaderType_e
	{
		TYPE_VERTEX = 0,
		TYPE_GEOMETRY = 1,
		TYPE_FRAGMENT = 2,

		MAX_SHADER_TYPES = 3,
	};
};



#endif	// __SHADER_H_ICNLDUDED__

