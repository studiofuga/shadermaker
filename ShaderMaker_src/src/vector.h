//=============================================================================
/** @file		vector.h
 *
 * Defines some vector helper classes.
 *
	@internal
	created:	2007-11-06
	last mod:	2008-01-26

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __VECTOR_H_INCLUDED__
#define __VECTOR_H_INCLUDED__

#include <math.h>


//=============================================================================
//	4D vector
//=============================================================================

/** 4D Vector.
 *
 * A vector class with self-explaining members.
 */
class Vector4D
{
public:
	/** Constructs a 4D verctor with all components set to zero.
	 */
	Vector4D( void ) : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
	{
	}

	/** Constructs a 3D vector from given component values.
	 */
	Vector4D( float X, float Y, float Z, float W ) : x(X), y(Y), z(Z), w(W)
	{
	}

	/** Constructs a 4D vector from a given float[4] array.
	 */
	Vector4D( const float* xyzw ) : x(xyzw[0]), y(xyzw[1]), z(xyzw[2]), w(xyzw[3])
	{
	}

	/** Tests wether two vectors are equal. */
	inline int operator==(const Vector4D& v) const { return (x==v.x && y==v.y && z==v.z && w==v.w); }

	/** Tests wether two vectors are different. */
	inline int operator!=(const Vector4D& v) const { return !(*this == v); }

	/** Returns the vector in reverse direction with same length. */
	inline Vector4D operator-(void)					const { return Vector4D(-x,-y,-z,-w); }

	/** Returns the sum of two vectors. */
	inline Vector4D operator+(const Vector4D& v)	const { return Vector4D(x+v.x,y+v.y,z+v.z,w+v.w); }

	/** Subtracts two vectors.
	 * @return this - v.
	 */
	inline Vector4D operator-(const Vector4D& v)	const { return Vector4D(x-v.x,y-v.y,z-v.z,w-v.w); }

	/** Multiplies the vector with a scalar.
	  * @param f Scaling factor.
	  * @return The scaled vector.
	  */
	inline Vector4D operator*(const float f)		const { return Vector4D(x*f,y*f,z*f,w*f); }

	/** Computes the dot product between the two vectors. */
	inline float operator*(const Vector4D & v) const { return x*v.x + y*v.y + z*v.z + w*v.w; }

	/** Allows accessing the components via index.
	 * @pre Assumes the compiler places x,y,z,w in this sequence without padding.
	 */
	inline const float* toFloatPointer( void ) const { return &x; }

	/** Multiplies the i-th componenent of *this with the i-th component of v. */
	inline Vector4D modulate( const Vector4D & v ) const { return Vector4D( x*v.x, y*v.y, z*v.z, w*v.w ); }

	/** Returnes the squared length of the vector. */
	inline float lengthSq( void ) const { return x*x + y*y + z*z + w*w; }

	/** Clamps each component to the range [0,1]. */
	inline void colorNormalize( void )
	{
		if( x < 0.0f ) x = 0.0f;
		if( x > 1.0f ) x = 1.0f;
		if( y < 0.0f ) y = 0.0f;
		if( y > 1.0f ) y = 1.0f;
		if( z < 0.0f ) z = 0.0f;
		if( z > 1.0f ) z = 1.0f;
		if( w < 0.0f ) w = 0.0f;
		if( w > 1.0f ) w = 1.0f;
	}

	float x; ///< X component.
	float y; ///< Y component.
	float z; ///< Z component.
	float w; ///< W component.
};


//=============================================================================
//	3D Vector
//=============================================================================


/** 3D Vector.
 * A vector class with self-explaining members.
 */
class Vector3D
{
public:

	/** Constructs a vector object from given components */
	Vector3D( float X=0.0f, float Y=0.0f, float Z=0.0f ) : x(X), y(Y), z(Z)
	{
	}

	/** Constructs a 2D vector fromthe first 3 components of a 4D vector. */
	Vector3D( const Vector4D & v ) : x(v.x), y(v.y), z(v.z)
	{
	}

	/** Adds wto vectors. */
	inline Vector3D operator+(const Vector3D &v) const { return Vector3D(x+v.x,y+v.y,z+v.z); }

	/** Subtracts two vectors.
	 * @return this - v.
	 */
	inline Vector3D operator-(const Vector3D &v) const { return Vector3D(x-v.x,y-v.y,z-v.z); }

	/** Multiplies the vector with a scalar.
	  * @param f Scaling factor.
	  * @return The scaled vector.
	  */
	inline Vector3D operator*(const float f) const { return Vector3D(x*f,y*f,z*f); }

	/** Allows accessing the components via index.
	 * @pre Assumes the compiler places x,y,z,w in this sequence without padding.
	 */
	inline const float* toFloatPointer( void ) const { return &x; }

	/** Returns the quared vector length. */
	inline float lengthSq( void ) const
	{
		return x*x + y*y + z*z;
	}

	/** Returns a vector with the same direction, but length 1.0.
	 * If this is called on a Null-Vector, a Null-Vector is returned.
	 */
	inline Vector3D normalize( void ) const
	{
		float len = lengthSq();
		if( len > 0.0f )
		{
			return (*this) * ( 1.0f / sqrtf( len ) );
		}
		return Vector3D( 0,0,0 );
	}


	/** Calculates the dot product.
	 * @param v Seconds vector.
	 */
	inline float dotProduct( const Vector3D & v ) const
	{
		return x*v.x + y*v.y + z*v.z;
	}


	/** Caculates the corss procut this x v.
	 * @param v Second vector.
	 * @return This x V.
	 */
	inline Vector3D crossProduct( const Vector3D & v ) const
	{
		return Vector3D(
			y * v.z - v.y * z,
			z * v.x - v.z * x,
			x * v.y - v.x * y
			);
	}


	/** Returns the vector with the absolute of each component.
	 */
	inline Vector3D absolute( void ) const
	{
		return Vector3D( (float)fabs( x ),
						 (float)fabs( y ),
						 (float)fabs( z ) );
	}


	/** Returns the maximum of the absolute of each component.
	 */
	inline float absoluteCoordMaximum( void ) const
	{
		float ax = (float)fabs( x );
		float ay = (float)fabs( y );
		float az = (float)fabs( z );

		if( ax > ay )
		{
			return ( ax > az ) ? ax : az;
		}

		// now: ax <= ay
		return ( ay > az ) ? ay : az;
	}

	float x; ///< X component.
	float y; ///< Y component.
	float z; ///< Z component.
};


//=============================================================================
//	2D Vector
//=============================================================================

/** 2D vector
 *
 * A vector class with self-explaining members.
 */
class Vector2D
{
public:

	/** Constructs a vector form given components. */
	Vector2D( float X=0.0f, float Y=0.0f ) : x(X), y(Y)
	{
	}

	/** Adds two vectors. */
	inline Vector2D operator+(const Vector2D &v) const { return Vector2D(x+v.x,y+v.y); }

	/** Subtracts two vectors.
	 * @return this - v.
	 */
	inline Vector2D operator-(const Vector2D &v) const { return Vector2D(x-v.x,y-v.y); }

	/** Multiplies the vector with a scalar.
	  * @param f Scaling factor.
	  * @return The scaled vector.
	  */
	inline Vector2D operator*(const float f) const { return Vector2D(x*f,y*f); }

	/** Allows accessing the components via index.
	 * @return The address of the X component.
	 * @pre Assumes the compiler places x,y,z,w in this sequence without padding.
	 */
	inline const float* toFloatPointer( void ) const { return &x; }

	float x; ///< X component
	float y; ///< Y component
};


//=============================================================================
//	4x4 matrix
//=============================================================================

/** This as a 4x4 column major order matrix.
 * This class can be used to pass around OpenGL style matrices.
 */
class Matrix4x4
{
public:
	/** Constructs an identity 4x4 matrix.
	 */
	Matrix4x4( void )
	{
		for( int i = 0 ; i < 16 ; i++ )
			m[i] = 0.0f;
		m[0] = m[5] = m[10] = m[15] = 1.0f;
	}


	/** Multiplies a 4x4 matrix with a 4x1 matrix.
	 */
	inline Vector4D operator*( const Vector4D & v ) const
	{
		// m contains a column major order matrix!
		return Vector4D(
			m[0] * v.x + m[4] * v.y + m[ 8] * v.z + m[12] * v.w,
			m[1] * v.x + m[5] * v.y + m[ 9] * v.z + m[13] * v.w,
			m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
			m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w );
	}


	/** Sets the translational part of the matrix.
	 * Because the matrix is column major oder, it sets the elements 12(x), 13(y) and 14(z).
	 */
	inline void setTranslationVector( const Vector3D & t )
	{
		m[12] = t.x;
		m[13] = t.y;
		m[14] = t.z;
	}

	/** Allows direct access to the matrix elements.
	 * You should only use this for reading/writing matrices from/to OpenGL state.
	 * @return pointer to an array of 16 floats representing a 4x4 column major order matrix.
	 */
	float* toFloatPointer( void ) { return m; }

	/** Allows direct access to the matrix elements.
	 * You should only use this for writing matrices to OpenGL state.
	 * @return pointer to an array of 16 floats representing a 4x4 column major order matrix.
	 */
	const float* toConstFloatPointer( void ) const { return m; }

private:
	float m[ 16 ]; // this is the 4x4 column major order matrix.
};


//=============================================================================
//	type name shortcuts
//=============================================================================

typedef Vector4D  vec4_t; ///< Shortcut for Vector4D
typedef Vector3D  vec3_t; ///< Shortcut for Vector3D
typedef Vector2D  vec2_t; ///< Shortcur for Vector2D
typedef Matrix4x4 mat4_t; ///< Shortcut for Matrix4x4


#endif	// __VECTOR_H_INCLUDED__
