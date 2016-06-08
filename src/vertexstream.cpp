//=============================================================================
/** @file		vertexstream.cpp
 *
 * Implements the vertex buffer class
 *
	@internal
	created:	2007-12-14
	last mod:	2008-02-02

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include "application.h"
#include "vertexstream.h"


//=============================================================================
//	IVertexStream implementation
//=============================================================================


/** Implementation of the IVertexStream interface.
 * The methods work like described in IVertexStream.
 */
class CVertexStream : public IVertexStream
{
public:
	/** Construcs a vertex stream objecct with space for a given number of vertices.
	 * @param numVertices Number of vertices this stream can contains.
	 */
	CVertexStream( int numVertices );
	virtual ~CVertexStream( void );

	// computes tanget space basis 
	// -> works only with individual triangles!!!
	void coumputeTangentVectors( void );

	// bounding radius
	float computeBoundingRadius( void );

	// rendering
	void render( int primitiveType, const vec4_t * overrideColor,
				 const VertexAttribLocations * attribs );
	void renderNormals( void );
	void renderTangentVectors( void );

	// vertex arrays
	vec3_t*	v( void ) { return m_vertices; }
	vec3_t*	n( void ) { return m_normals; }
	vec2_t*	t( void ) { return m_texCoords; }
	vec4_t*	c( void ) { return m_colors; }
	vec3_t* tan1( void ) { return m_tangents; }
	vec3_t* tan2( void ) { return m_bitangents; }

private:

	/** The custom vertex attributes, currently ony the tangent space vectors. */
	typedef struct tanSpace_s {
		vec3_t	tangent;
		vec3_t	bitangent;
	} tanSpace_t;

	int			m_numVertices;
	vec3_t*		m_vertices;
	vec3_t*		m_normals;
	vec2_t*		m_texCoords;
	vec4_t*		m_colors;
	vec3_t*		m_tangents;
	vec3_t*		m_bitangents;
};


// construction
CVertexStream::CVertexStream( int numVertices )
 : m_numVertices( numVertices )
{
	m_vertices		= new vec3_t[ m_numVertices ];
	m_normals		= new vec3_t[ m_numVertices ];
	m_texCoords		= new vec2_t[ m_numVertices ];
	m_colors		= new vec4_t[ m_numVertices ];
	m_tangents		= new vec3_t[ m_numVertices ];
	m_bitangents	= new vec3_t[ m_numVertices ];
}

// destruction
CVertexStream::~CVertexStream( void )
{
	SAFE_DELETE_ARRAY( m_vertices );
	SAFE_DELETE_ARRAY( m_normals );
	SAFE_DELETE_ARRAY( m_texCoords );
	SAFE_DELETE_ARRAY( m_colors );
	SAFE_DELETE_ARRAY( m_tangents );
	SAFE_DELETE_ARRAY( m_bitangents );
}


/*
========================
IVertexStream::create
========================
*/
IVertexStream* IVertexStream::create( int numVertices )
{
	return new CVertexStream( numVertices );
}


/*
========================
computeBoundingRadius
========================
*/
float CVertexStream::computeBoundingRadius( void )
{
	float radius = 0.0f;

	for( int i = 0 ; i < m_numVertices ; i++ )
	{
		float lsq = m_vertices[ i ].lengthSq();
		radius = qMax( radius, lsq );
	}

	return sqrt( radius );
}


/*
========================
render
========================
*/
void CVertexStream::render( int primitiveType, const vec4_t * overrideColor,
						    const VertexAttribLocations * attribs )
{
	// enable arrays
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	// primary color
	if( overrideColor != NULL ) {
		glColor4fv( overrideColor->toFloatPointer() );
		glDisableClientState( GL_COLOR_ARRAY );
	} else {
		glEnableClientState( GL_COLOR_ARRAY );
	}

	// set pointers
	glVertexPointer  ( 3, GL_FLOAT, 0, m_vertices );
	glNormalPointer  (    GL_FLOAT, 0, m_normals );
	glTexCoordPointer( 2, GL_FLOAT, 0, m_texCoords );
	glColorPointer   ( 4, GL_FLOAT, 0, m_colors );

	// tangent space matrix, X
	if( attribs != NULL && attribs->tangent != -1 ) {
		glVertexAttribPointer( attribs->tangent, 3, GL_FLOAT, true, sizeof(vec3_t), m_tangents );
		glEnableVertexAttribArray( attribs->tangent );
	}

	// tangent space matrix, Y
	if( attribs != NULL && attribs->bitangent != -1 ) {
		glVertexAttribPointer( attribs->bitangent, 3, GL_FLOAT, true, sizeof(vec3_t), m_bitangents );
		glEnableVertexAttribArray( attribs->bitangent );
	}

	// draw it
	glDrawArrays( primitiveType, 0, m_numVertices );

	// clean up state
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );

	// disable custom attribs
	if( attribs != NULL && attribs->tangent != -1 )
		glDisableVertexAttribArray( attribs->tangent );

	if( attribs != NULL && attribs->bitangent != -1 )
		glDisableVertexAttribArray( attribs->bitangent );
}


/*
========================
renderTangentVectors
========================
*/
void CVertexStream::renderTangentVectors( void )
{
	float length = 0.1f;

	glBegin( GL_LINES );

	for( int i = 0 ; i < m_numVertices ; i++ )
	{
		// tangent
		glColor3f( 1,0,0 );
		glVertex3fv( m_vertices[i].toFloatPointer() );
		glVertex3fv( ( m_vertices[i] + m_tangents[i] * length ).toFloatPointer() );

		// bitangent
		glColor3f( 0,1,0 );
		glVertex3fv( m_vertices[i].toFloatPointer() );
		glVertex3fv( ( m_vertices[i] + m_bitangents[i] * length ).toFloatPointer() );

		// normal
		glColor3f( 0,0,1 );
		glVertex3fv( m_vertices[i].toFloatPointer() );
		glVertex3fv( ( m_vertices[i] + m_normals[i] * length ).toFloatPointer() );
	}

	glEnd();
}


/*
========================
renderNormals
========================
*/
void CVertexStream::renderNormals( void )
{
	glBegin( GL_LINES );

	for( int i = 0 ; i < m_numVertices ; i++ )
	{
		const vec3_t & n = m_normals[ i ];
		float x = fabs( n.x );
		float y = fabs( n.y );
		float z = fabs( n.z );

		// select max. component as color
		if( x > y && x > z ) {
			glColor3f( 1,0,0 );
		} else if( y > x && y > z ) {
			glColor3f( 0,1,0 );
		} else if( z > x && z > y ) {
			glColor3f( 0,0,1 );
		} else { // two components equal
			glColor3f( 1,1,1 );
		}

		glVertex3fv( m_vertices[i].toFloatPointer() );
		glVertex3fv( ( m_vertices[i] + n * 0.3f ).toFloatPointer() );
	}

	glEnd();
}


/*
========================
computeTangentVectors
========================
*/
void CVertexStream::coumputeTangentVectors( void )
{
	// assumes triangles!
	int numTriangles = m_numVertices / 3;

	for( int i = 0 ; i < numTriangles ; i++ )
	{
		vec3_t e1 = m_vertices [3*i+1] - m_vertices [3*i];
		vec3_t e2 = m_vertices [3*i+2] - m_vertices [3*i];
		vec2_t t1 = m_texCoords[3*i+1] - m_texCoords[3*i];
		vec2_t t2 = m_texCoords[3*i+2] - m_texCoords[3*i];

		float length = t1.y * t2.x - t1.x *t2.y;
		if( fabs(length) > 0.000001 ) // triangle not degenerated
		{
			// solve linear equations
			vec3_t planeTangent = ( e2 * t1.y - e1 * t2.y );

			// adjust tangents to the vertex normal
			for( int k = 0 ; k < 3 ; k++ )
			{
				// assumed to be normalized.
				const vec3_t & normal = m_normals[3*i+k];

				// orthogonalize
				vec3_t tangent = planeTangent - normal * planeTangent.dotProduct( normal );
				tangent = tangent.normalize();

				m_tangents  [3*i+k] = tangent;
				m_bitangents[3*i+k] = tangent.crossProduct( normal );
			}
		}
	}
}
