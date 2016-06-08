//=============================================================================
/** @file		uniform.h
 *
 * Defines a uniform variable management class
 *
	@internal
	created:	2007-11-16
	last mod:	2007-12-16

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __UNIFORM_H_INCLUDED__
#define __UNIFORM_H_INCLUDED__

#include <QtCore/QString>
#include <QtOpenGL/QGLContext>


//=============================================================================
//	CUniform  - contains info about a programs's uniform variable
//=============================================================================

/** Stores infos about uniform variables of a GLSL program.
 * This class is capable of storing uniform variable data of
 * different types and formats used in GLSL program. It stores the
 * data itself and the metadata, like variable name, type and location.
 * It also provides methods that return meta information about
 * those meta informations ( example: getBaseType() ).
 *
 * This class supports bool, int and float variables with up to 4x4 elements.
 * The methods for accessing the data of these types are exclusive to its type.
 * For example, results are undefined if the uniform type is int and
 * the method setValueAsFloat is called. These types are also called base types,
 * because they are the scalars used in vectors an matrix uniforms.
 *
 * A CUniform object can be accessed like a matrix and like a vector.
 * the getValue and setValue methods treat it like a 4 component vector,
 * even if only one component is available! Accessing the other components is possible,
 * but they are not passed to OpenGL via applyToGL. Matrices are also treated
 * like vectors by accessing only the first column of the matrix.
 * To access the other columns you need to extract/insert the individual
 * column with the getColumnVector() and setColumnVector() methods.
 */
class CUniform
{
public:
	CUniform( const QString & name=QString(), int type=0, int location=-1 );
	CUniform( const CUniform & u, int location );
	virtual ~CUniform( void );

	/** Uniform base type.
	 * Since uniforms can be vectors and matrices, this defines the
	 * type if the scalars stored in those vectors and matrices.
	 */
	enum baseType_e
	{
		BASE_TYPE_BAD = 0,
		BASE_TYPE_BOOL,
		BASE_TYPE_INT,
		BASE_TYPE_FLOAT,
		BASE_TYPE_SAMPLER,
	};

	/** Returns the name of the uniform variable. */
	const QString & getName( void ) const { return m_name; }

	/** Returns the OpenGL type of the uniform variable. */
	int getType( void ) const { return m_type; }

	/** Returns the name string of the OpenGL type of this uniform. */
	QString getTypeName( void ) const { return getTypeNameString( m_type ); }

	/** Returns the location of this uniform. */
	int getLocation( void ) const { return m_location; }

	// meta infos.
	int getComponentCount( void ) const;
	baseType_e getBaseType( void ) const;

	// matrices
	bool     isMatrix( void ) const;
	int      getColumnCount( void ) const;
	CUniform getColumnVector( int column ) const;
	void     setColumnVector( int column, const CUniform & u );

	// content access
	// -> only supports vectors with [1,4] components!
	bool   getValueAsBool ( int component ) const;
	int    getValueAsInt  ( int component ) const;
	double getValueAsFloat( int component ) const;
	void   setValueAsBool ( int component, bool value );
	void   setValueAsInt  ( int component, int value );
	void   setValueAsFloat( int component, double value );

	// passes its data directly to the GL
	void applyToGL( void );

	// helpers
	static QString getTypeNameString( int type );

private:

	/** The actual data container.
	 * @internal
	 *   - Must match for each basic type!
	 */
	typedef union dataUnit_u {
		GLfloat	_float	[ 16 ]; ///< up to mat4, in column major order!
		GLint	_int	[ 4 ];  ///< up to ivec4 and bvec4
	} dataUnit_t;

	QString		m_name;
	int			m_type; // GL_xxx type identifier
	int			m_location;
	dataUnit_t	m_data;
};


//=============================================================================
//	IUniformState - manages uniforms
//=============================================================================

/** Stores a list of uniforms.
 * The uniforms an be read and written.
 * Uniforms can be accessed with a zero based index.
 */
class IUniformState
{
public:
	virtual ~IUniformState( void ) {}

	/** Returns the number of uniforms stored in this object.
	 * The returned value is always >= 0.
	 * @return The highest possible uniform index + 1.
	 */
	virtual int getActiveUniforms( void ) = 0;

	/** Returns an indexed uniform.
	 * If the index is out of range, a CUniform object with undefined
	 * content will be returned.
	 * @param index Zero based index to a uniform. The maximum
	 *        possible index is n-1 where n is the value returned by getActiveUniforms().
	 * @return Uniform at the given index.
	 */
	virtual CUniform getUniform( int index ) = 0;

	/** Sets the uniform at a given index.
	 * If the index is out of range, this call has no effect.
	 * If the new uniform has a different name or a different type
	 * or a different location like the uniform currently stored,
	 * this call has no effect.
	 * @param index Zero based index to a uniform. The maximum
	 *        possible index is n-1 where n is the value returned by getActiveUniforms().
	 * @param u The value the indexed uniform is set to.
	 */
	virtual void setUniform( int index, const CUniform & u ) = 0;
};



#endif	// __UNIFORM_H_INCLUDED__
