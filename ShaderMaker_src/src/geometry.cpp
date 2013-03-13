//=============================================================================
/** @file		geometry.cpp
 *
 * Implements the test model geometry.
 *
	@internal
	created:	2007-11-24
	last mod:	2008-01-07

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include "application.h"
#include "model.h"
#include "vertexstream.h"

//=============================================================================
//	IModel implementation
//=============================================================================

/*
========================
primitiveTypeName
========================
*/
QString IModel::primitiveTypeName( int primitiveType )
{
	switch( primitiveType )
	{
	case GL_POINTS:			return QString( "GL_POINTS" ); break;
	case GL_LINES:			return QString( "GL_LINES:" ); break;
	case GL_LINE_STRIP:		return QString( "GL_LINE_STRIP" ); break;
	case GL_LINE_LOOP:		return QString( "GL_LINE_LOOP" ); break;
	case GL_TRIANGLES:		return QString( "GL_TRIANGLES" ); break;
	case GL_TRIANGLE_STRIP:	return QString( "GL_TRIANGLE_STRIP" ); break;
	case GL_TRIANGLE_FAN:	return QString( "GL_TRIANGLE_FAN" ); break;
	case GL_QUADS:			return QString( "GL_QUADS" ); break;
	case GL_QUAD_STRIP:		return QString( "GL_QUAD_STRIP" ); break;
	case GL_POLYGON:		return QString( "GL_POLYGON" ); break;
	}

	return QString( "< bad primitive type %1>" ).arg( primitiveType );
}


//=============================================================================
//	CBaseModdel
//=============================================================================

/** Implementation of IModel.
 * There are some additinal functions for geometry construction available
 * in this class.
 */
class CBaseModel : public IModel
{
public:
	// construction
	CBaseModel( const QString & name, int primitiveType, 
				const vec3_t & mins, const vec3_t & maxs,
				float boundingRadius, IVertexStream* vertices );
	virtual ~CBaseModel( void ); ///< Destructor.

	// IModel interface
	QString getName( void ) { return m_name; }
	int     getPrimitiveType( void ) { return m_primitiveType; }
	QString getPrimitiveTypeName( void );
	float   getBoundingRadius( void ) { return m_boundingRadius; }
	void	getBoundingBox( vec3_t & mins, vec3_t & maxs ) { mins = m_mins; maxs = m_maxs; }

	void    render( const VertexAttribLocations* attribs, const vec4_t * overrideColor );
	void    renderNormals( void );
	void	renderTangents( void );

	/** Builds a tesseleted plane.
	 * This function recursively builds a plane of triangle pairs.
	 * In fact, the plane is a quad.
	 * Each recursion level splits the quad into four sub-qurads,
	 * the upper-left, upper-right, lower-left and lower-right quads.
	 * These quads are created by calling the buildPlane functions
	 * with level-1. The base level is 1.
	 * \n\n
	 * The parameters v,n,t and c point to arrays of n elements, where
	 * n = 6 * 4^level. This means every quad is created by two triangles,
	 * which require 6 vertices. Because the quads are split each level,
	 * the number of quads increases by factor 4 at each level. This results
	 * in 4^level. The caller of this function must make sure that enough
	 * space is allocated to hold the triangles for the requested recursion level.
	 * \n\n
	 * The parameters holds the vetex normal that is assigned to every
	 * vertex this function produces.
	 * \n\n
	 * The parameters mainVertices, mainTexCoords and mainColors point to
	 * arrays with four element wich hold the vertex attributes for the
	 * lower-left, lower-right, uppler-left, upper-right corner of the quad,
	 * in that order. Thses values are linear interpolated when goint into recursion.
	 *
	 * @param v Vertex position array.
	 * @param n Normal array.
	 * @param t TexCoord array.
	 * @param c Color array.
	 * @param mainVertices Input position.
	 * @param normal Surface normal.
	 * @param mainTexCoords Input texture coordinates.
	 * @param mainColors Input colors.
	 * @param level Recursion level, must be >= 1.
	 */
	static void buildPlane( vec3_t* v, vec3_t* n, vec2_t* t, vec4_t* c,
							const vec3_t* mainVertices, const vec3_t & normal,
							const vec2_t* mainTexCoords, const vec4_t* mainColors,
							int level );

	/** Helper method for buildPlane.
	 * It calculates the amount of vertices in a plane for a recusion level.
	 * @param level Recusion level.
	 * @return Vertex count for that level.
	 * @see buildPlane.
	 */
	static int vertexCountForPlaneLevel( int level );

private:

	QString			m_name;
	int				m_primitiveType;
	vec3_t			m_mins, m_maxs; // bounding box
	float			m_boundingRadius;
	IVertexStream*	m_vertices;
};


/** Constructs a model object.
 * It takes a vertex stream and its metadata and stores it.
 * The vertex stream is deleted when this object is destroyed.
 * @param name Model name.
 * @param primitiveType OpenGL primitive type of this model.
 * @param mins Bounding box mins.
 * @param maxs Bounding box maxs.
 * @param boundingRadius Bounding sphere radius.
 * @param vertices IVertexStream object that stores the model's geometry.
 */
CBaseModel::CBaseModel( const QString & name, int primitiveType,
						const vec3_t & mins, const vec3_t & maxs,
					    float boundingRadius, IVertexStream* vertices )
 : m_name( name ), m_primitiveType( primitiveType ),
   m_mins( mins ), m_maxs( maxs ),
   m_boundingRadius( boundingRadius ), m_vertices( vertices )
{
}

CBaseModel::~CBaseModel( void )
{
	SAFE_DELETE( m_vertices );
}


/*
========================
render
========================
*/
void CBaseModel::render( const VertexAttribLocations * attribs, const vec4_t * overrideColor )
{
	if( m_vertices != NULL )
	{
		m_vertices->render( m_primitiveType, overrideColor, attribs );
	}
}


/*
========================
renderNormals
========================
*/
void CBaseModel::renderNormals( void )
{
	if( m_vertices != NULL )
	{
		m_vertices->renderNormals();
	}
}


/*
========================
renderTangents
========================
*/
void CBaseModel::renderTangents( void )
{
	if( m_vertices != NULL )
	{
		m_vertices->renderTangentVectors();
	}
}


/*
========================
getPrimitiveTypeName
========================
*/
QString CBaseModel::getPrimitiveTypeName( void )
{
	return primitiveTypeName( m_primitiveType );
}


/*
========================
vertexCountForPlaneLevel

 see buildPlane()
========================
*/
int CBaseModel::vertexCountForPlaneLevel( int level )
{
	int count = 1;

	for( int i = 1 ; i < level ; i++ )
	{
		count = count * 4;
	}

	return 6 * count;
}


/*
========================
buildPlane

 Recursively builds a plane.
 v,n,t,c will hold the result and must be allocated by the caller.
 level controls how often the plane is split along both axis.
 level must be >= 1.
 The data order is (-1,-1), (+1,-1), (-1,+1), (+1,+1) both in input
 and output buffers, where -1 represents the minimum and +1 the maximum.
 It uses two triangles for each rectangle.
 The buffer size in components is 6 * NumOfRects(level) = 6 * 4 ^ ( level - 1 ).
========================
*/
void CBaseModel::buildPlane( vec3_t* v, vec3_t* n, vec2_t*t, vec4_t* c,
							 const vec3_t* mainVertices, const vec3_t & normal,
							 const vec2_t* mainTexCoords, const vec4_t* mainColors,
							 int level )
{
	//
	// recursion case
	//
	if( level > 1 )
	{
		// temp buffers for interpolation
		vec3_t tmpV[ 4 ];
		vec2_t tmpT[ 4 ];
		vec4_t tmpC[ 4 ];
		int skip = 0;
		int skipIncrement = vertexCountForPlaneLevel( level - 1 );

		//
		// (-1,-1) rectangle
		//
		tmpV[0] =   mainVertices[0];
		tmpV[1] = ( mainVertices[1] + mainVertices[0] ) * 0.5f;
		tmpV[2] = ( mainVertices[2] + mainVertices[0] ) * 0.5f;
		tmpV[3] = ( mainVertices[3] + mainVertices[0] ) * 0.5f;
		tmpT[0] =   mainTexCoords[0];
		tmpT[1] = ( mainTexCoords[1] + mainTexCoords[0] ) * 0.5f;
		tmpT[2] = ( mainTexCoords[2] + mainTexCoords[0] ) * 0.5f;
		tmpT[3] = ( mainTexCoords[3] + mainTexCoords[0] ) * 0.5f;
		tmpC[0] =   mainColors[0];
		tmpC[1] = ( mainColors[1] + mainColors[0] ) * 0.5f;
		tmpC[2] = ( mainColors[2] + mainColors[0] ) * 0.5f;
		tmpC[3] = ( mainColors[3] + mainColors[0] ) * 0.5f;

		// recursion step
		buildPlane( v,n,t,c, tmpV, normal, tmpT, tmpC, level-1 );

		//
		// (+1,-1) rectangle
		//
		tmpV[0] = ( mainVertices[1] + mainVertices[0] ) * 0.5f;
		tmpV[1] =   mainVertices[1];
		tmpV[2] = ( mainVertices[3] + mainVertices[0] ) * 0.5f;
		tmpV[3] = ( mainVertices[3] + mainVertices[1] ) * 0.5f;
		tmpT[0] = ( mainTexCoords[1] + mainTexCoords[0] ) * 0.5f;
		tmpT[1] =   mainTexCoords[1];
		tmpT[2] = ( mainTexCoords[3] + mainTexCoords[0] ) * 0.5f;
		tmpT[3] = ( mainTexCoords[3] + mainTexCoords[1] ) * 0.5f;
		tmpC[0] = ( mainColors[1] + mainColors[0] ) * 0.5f;
		tmpC[1] =   mainColors[1];
		tmpC[2] = ( mainColors[3] + mainColors[0] ) * 0.5f;
		tmpC[3] = ( mainColors[3] + mainColors[1] ) * 0.5f;

		// recursion step
		skip += skipIncrement;
		buildPlane( v+skip, n+skip, t+skip, c+skip, tmpV, normal, tmpT, tmpC, level - 1 );

		//
		// (-1,+1) rectangle
		//
		tmpV[0] = ( mainVertices[2] + mainVertices[0] ) * 0.5f;
		tmpV[1] = ( mainVertices[3] + mainVertices[0] ) * 0.5f;
		tmpV[2] =   mainVertices[2];
		tmpV[3] = ( mainVertices[3] + mainVertices[2] ) * 0.5f;
		tmpT[0] = ( mainTexCoords[2] + mainTexCoords[0] ) * 0.5f;
		tmpT[1] = ( mainTexCoords[3] + mainTexCoords[0] ) * 0.5f;
		tmpT[2] =   mainTexCoords[2];
		tmpT[3] = ( mainTexCoords[3] + mainTexCoords[2] ) * 0.5f;
		tmpC[0] = ( mainColors[2] + mainColors[0] ) * 0.5f;
		tmpC[1] = ( mainColors[3] + mainColors[0] ) * 0.5f;
		tmpC[2] =   mainColors[2];
		tmpC[3] = ( mainColors[3] + mainColors[2] ) * 0.5f;

		// recursion step
		skip += skipIncrement;
		buildPlane( v+skip, n+skip, t+skip, c+skip, tmpV, normal, tmpT, tmpC, level - 1 );

		//
		// (+1,+1) rectangle
		//
		tmpV[0] = ( mainVertices[3] + mainVertices[0] ) * 0.5f;
		tmpV[1] = ( mainVertices[3] + mainVertices[1] ) * 0.5f;
		tmpV[2] = ( mainVertices[3] + mainVertices[2] ) * 0.5f;
		tmpV[3] =   mainVertices[3];
		tmpT[0] = ( mainTexCoords[3] + mainTexCoords[0] ) * 0.5f;
		tmpT[1] = ( mainTexCoords[3] + mainTexCoords[1] ) * 0.5f;
		tmpT[2] = ( mainTexCoords[3] + mainTexCoords[2] ) * 0.5f;
		tmpT[3] =   mainTexCoords[3];
		tmpC[0] = ( mainColors[3] + mainColors[0] ) * 0.5f;
		tmpC[1] = ( mainColors[3] + mainColors[1] ) * 0.5f;
		tmpC[2] = ( mainColors[3] + mainColors[2] ) * 0.5f;
		tmpC[3] =   mainColors[3];

		skip += skipIncrement;
		buildPlane( v+skip, n+skip, t+skip, c+skip, tmpV, normal, tmpT, tmpC, level - 1 );
	}


	//
	// base case.
	//
	else if( level == 1 )
	{
		// maps input to output:
		//  out[ i ] = in[ map[ i ] ]
		static const int map[ 6 ] =
		{
			0, 1, 2, // first triangle
			3, 2, 1, // second triangle
		};

		for( int i = 0 ; i < 6 ; i++ )
		{
			v[ i ] = mainVertices[ map[i] ];
			n[ i ] = normal;
			t[ i ] = mainTexCoords[ map[i] ];
			c[ i ] = mainColors[ map[i] ];
		}
	}
}


//=============================================================================
//	cube test model
//=============================================================================

//======================
/** Creates a cube model.
 * The model is an RGB cube, located in the volume (-1,-1,-1) to (+1,+1,+1).
 * The colors are those of the RGB cube with X=red, Y=green, Z=blue.
 * The components intensities increase with increasing vertex coordinate.
 * The texture coordinates define the full texture image on every face of the cube.
 * The face normals point out of the cube.
 */
//======================
IModel* IModel::createCube( void )
{
	int level = 4;
	int vertexCount = CBaseModel::vertexCountForPlaneLevel( level );
	IVertexStream* v = IVertexStream::create( vertexCount * 6 );

	// vertices
	static vec3_t vIn[6][4] =
	{
		// front
		{
			vec3_t( -1, -1, +1 ), vec3_t( +1,-1, +1 ),
			vec3_t( -1, +1, +1 ), vec3_t( +1,+1, +1 ),
		},

		// back
		{
			vec3_t( +1, -1, -1 ), vec3_t( -1,-1, -1 ),
			vec3_t( +1, +1, -1 ), vec3_t( -1,+1, -1 ),
		},

		// left
		{
			vec3_t( -1, +1, +1 ), vec3_t( -1, +1, -1 ),
			vec3_t( -1, -1, +1 ), vec3_t( -1, -1, -1 ),
		},

		// right
		{
			vec3_t( +1, +1, -1 ), vec3_t( +1, +1, +1 ),
			vec3_t( +1, -1, -1 ), vec3_t( +1, -1, +1 ),
		},

		// top
		{
			vec3_t( +1, +1, -1 ), vec3_t( -1, +1, -1 ),
			vec3_t( +1, +1, +1 ), vec3_t( -1, +1, +1 ),
		},

		// bottom
		{
			vec3_t( +1, -1, +1 ), vec3_t( -1, -1, +1 ),
			vec3_t( +1, -1, -1 ), vec3_t( -1, -1, -1 ),
		},
	};

	// normals
	static vec3_t nIn[6] = 
	{
		vec3_t(  0, 0,+1 ), // front
		vec3_t(  0, 0,-1 ), // back
		vec3_t( -1, 0, 0 ), // left
		vec3_t( +1, 0, 0 ), // right
		vec3_t(  0,+1, 0 ), // top
		vec3_t(  0,-1, 0 ), // bottom
	};

	// texcoords
	static vec2_t tIn[6][4] =
	{
		// front
		{
			vec2_t( 0,1 ), vec2_t( 1,1 ),
			vec2_t( 0,0 ), vec2_t( 1,0 ),
		},

		// back
		{
			vec2_t( 0,1 ), vec2_t( 1,1 ),
			vec2_t( 0,0 ), vec2_t( 1,0 ),
		},

		// left
		{
			vec2_t( 1,0 ), vec2_t( 0,0 ),
			vec2_t( 1,1 ), vec2_t( 0,1 ),
		},

		// right
		{
			vec2_t( 1,0 ), vec2_t( 0,0 ),
			vec2_t( 1,1 ), vec2_t( 0,1 ),
		},

		// top
		{
			vec2_t( 1,0 ), vec2_t( 0,0 ),
			vec2_t( 1,1 ), vec2_t( 0,1 ),
		},

		// bottom
		{
			vec2_t( 0,1 ), vec2_t( 1,1 ),
			vec2_t( 0,0 ), vec2_t( 1,0 ),
		},
	};

	// colors
	static vec4_t cIn[6][4] =
	{
		// front
		{
			vec4_t( 0,0,1,1 ), vec4_t( 1,0,1,1 ),
			vec4_t( 0,1,1,1 ), vec4_t( 1,1,1,1 ),
		},

		// back
		{
			vec4_t( 1,0,0,1 ), vec4_t( 0,0,0,1 ),
			vec4_t( 1,1,0,1 ), vec4_t( 0,1,0,1 ),
		},

		// left
		{
			vec4_t( 0,1,1,1 ), vec4_t( 0,1,0,1 ),
			vec4_t( 0,0,1,1 ), vec4_t( 0,0,0,1 ),
		},

		// right
		{
			vec4_t( 1,1,0,1 ), vec4_t( 1,1,1,1 ),
			vec4_t( 1,0,0,1 ), vec4_t( 1,0,1,1 ),
		},

		// top
		{
			vec4_t( 1,1,0,1 ), vec4_t( 0,1,0,1 ),
			vec4_t( 1,1,1,1 ), vec4_t( 0,1,1,1 ),
		},

		// bottom
		{
			vec4_t( 1,0,1,1 ), vec4_t( 0,0,1,1 ),
			vec4_t( 1,0,0,1 ), vec4_t( 0,0,0,1 ),
		},
	};

	// setup all cube sides
	for( int i = 0 ; i < 6 ; i++ )
	{
		int side = i * vertexCount;

		CBaseModel::buildPlane( v->v()+side, v->n()+side, v->t()+side, v->c()+side,
								vIn[i], nIn[i], tIn[i], cIn[i], level );
	}

	v->coumputeTangentVectors();

	return new CBaseModel( QString( "Cube" ), GL_TRIANGLES, 
				vec3_t( -1,-1,-1 ), vec3_t( 1,1,1 ), sqrtf(3.0f), v );
}


//=============================================================================
//	plane test model
//=============================================================================


//======================
/** Creates a tesselated quad located in the Z=0 plane.
 * The returned model has the color (1,1,1,1) and a the normal (0,0,1).
 * The vertex coordinates range form (-1,-1,0) to (+1,+1,0).
 * The texture coordinates cover th entire quad.
 */
//======================
IModel* IModel::createPlane( void )
{
	int level = 4;
	IVertexStream* v = IVertexStream::create( CBaseModel::vertexCountForPlaneLevel(level) );

	// vertices
	static vec3_t vIn[] =
	{
		vec3_t( -1,-1, 0 ), vec3_t( +1,-1, 0 ),
		vec3_t( -1,+1, 0 ), vec3_t( +1,+1, 0 ),
	};

	// texcoords
	static vec2_t tIn[] =
	{
		vec2_t( 0,1 ), vec2_t( 1,1 ),
		vec2_t( 0,0 ), vec2_t( 1,0 ),
	};

	// colors
	static vec4_t cIn[] =
	{
		vec4_t( 1,1,1,1 ), vec4_t( 1,1,1,1 ), // all white
		vec4_t( 1,1,1,1 ), vec4_t( 1,1,1,1 ),
	};

	CBaseModel::buildPlane( v->v(), v->n(), v->t(), v->c(),
							vIn, vec3_t(0,0,1), tIn, cIn,
							level );

	v->coumputeTangentVectors();

	return new CBaseModel( QString( "Plane" ), GL_TRIANGLES,
		vec3_t( -1,-1,0 ), vec3_t( 1,1,0 ), sqrtf( 2.0f ), v );
}


//=============================================================================
//	sphere test model
//=============================================================================


//======================
/** Creates an UV sphere.
 * The sphere is a list of single triangles that form a sphere.
 * The sphere is approximated by several rings, stacked from one pole to the other.
 * Each ring is a list of segments (quads), which are defined by two triangles.
 * As a result, the number of vertices required for the sphere is approx.
 * 3 * numTriangles = 3 * 2 * numQuads = 3 * 2 * numRings * numSegments.
 * The rings at the poles are created of triangles list to avoid degenerated triangles,
 * so the exact amount of vertices is 3 * 2 * ( numRings - 2 ) * numSegments + 3 * numSegments * ( 1 + 1 )
 * = 3 * 2 * numSegments * ( numRings - 1 ).
 * The vertex colors are interpolated between the north pole (red) and the equator (green)
 * and between the equator (green) and the south pole (blue).
 * The shere's texture coordiantes wrap the complete 2D texture image around the sphere,
 * which causes singularities on the poles and a seam on the edge shared by the first and the last
 * quad of each segment. The normals point out of the sphere.
 *
 * @param numRings Number of rings of the sphere.
 * @param numSegments Number of segments of the sphere.
 * @param radius Radius of the sphere.
 * @return An IModel that represents the sphere.
 */
//======================
IModel* IModel::createSphere( int numRings, int numSegments, float radius )
{
	int verticesPerQuad = 6; // must match the algorithm below!

	static const vec4_t colorNorth  ( 1,0,0,1 );
	static const vec4_t colorSouth  ( 0,0,1,1 );
	static const vec4_t colorEquator( 0,1,0,1 );

	// alloc space for numVertices = verticesPerQuad * numQuads
	IVertexStream* stream = IVertexStream::create( verticesPerQuad * numSegments * ( numRings - 1 ) );

	// get array pointers
	vec3_t* v = stream->v();
	vec3_t* n = stream->n();
	vec2_t* t = stream->t();
	vec4_t* c = stream->c();

	static const float pi = 4.0f * atanf( 1.0f );
	float stepNS = pi / float(numRings);    // north -> south
	float stepWE = 2.0f * pi / float(numSegments); // west ->east
	float alpha  = 0.5f * pi; // starting at top
	float V2 = 0.0f; // texture coord

	// for each ring
	for( int i = 0 ; i < numRings; i++ )
	{
		float oldalpha = alpha;
		alpha -= stepNS;

		float V1 = V2;
		V2 = float( i+1 ) / float( numRings );

		float sa1 = sin( oldalpha );
		float sa2 = sin( alpha );
		float ca1 = cos( oldalpha );
		float ca2 = cos( alpha );

		float beta = 1.0;

		// X texture coord, see note below.
		float U2 = 1.0f;

		// for each segment
		for( int j = 0 ; j < numSegments ; j++ )
		{
			float oldbeta = beta;
			beta += stepWE;

			float U1 = U2;
			U2 = 1.0f - float( j+1 ) / float( numSegments );

			float sb1 = sinf( oldbeta );
			float sb2 = sinf( beta );
			float cb1 = cosf( oldbeta );
			float cb2 = cosf( beta );

	//
	// Helper macro for creating sphere vertices.
	// Imagine the ring unrolled in a 2D cartesian coordinate system:
	//    +-+-+-+-+-+
	//    |\|\|\|\|\|
	//    +-+-+-+-+-+
	//  X points from right to left!
	//  Y points from top to bottom!
	//  First argument is the 'y' coord, second the 'x' coord.
	//
#define DEFINE_VERTEX(ring,seg) \
	/* setup normal / vertex position */ \
	*n = vec3_t( ca ## ring * cb ## seg, \
				 sa ## ring, \
				 ca ## ring * sb ## seg ); \
	/* setup vertex position */ \
	*v = (*n) * radius; \
	\
	/* setup color - interpolate along Y axis */ \
	*c = n->y < 0.0f ? \
		( colorNorth * (-(n->y)) + colorEquator * ( 1 + (n->y)) ) : \
		( colorSouth *   (n->y)  + colorEquator * ( 1 - (n->y)) ); \
	\
	/* setup texture coords */ \
	*t = vec2_t( U ## seg, V ## ring ); \
	\
	/* advance array pointers */ \
	v++; n++; t++; c++;


			// setup vertices for this ring
			if( i == 0 ) // north pole
			{
				DEFINE_VERTEX( 2,2 );
				DEFINE_VERTEX( 2,1 );
				DEFINE_VERTEX( 1,1 );
			}
			else if( i == numRings - 1 ) // south pole
			{
				DEFINE_VERTEX( 1,1 );
				DEFINE_VERTEX( 1,2 );
				DEFINE_VERTEX( 2,2 );
			}
			else
			{
				DEFINE_VERTEX( 1,1 );
				DEFINE_VERTEX( 1,2 );
				DEFINE_VERTEX( 2,2 );
				DEFINE_VERTEX( 2,2 );
				DEFINE_VERTEX( 2,1 );
				DEFINE_VERTEX( 1,1 );
			}

#undef DEFINE_VERTEX

		}
	}

	// setup tangent space vectors
	stream->coumputeTangentVectors();

	return new CBaseModel( QString( "Sphere" ), GL_TRIANGLES,
				vec3_t( -1,-1,-1 ), vec3_t( 1,1,1 ), radius, stream );
}


//=============================================================================
//	torus test model
//=============================================================================

//======================
/** Creates a torus.
 * The torus is a list of single triangles that form the torus.
 * The torus is a cylinder with radius2 deformed so that the top and the bottom of the cylinder are connected.
 * This cylinder is located on a circle with radius1 around the origin.
 * The cylinder is a stack of several rings.
 * Each ring is created of several segments (quads), which are defined by two triangles.
 * The required amount of vertices is 3 * numTriangles = 3 * 2 * numQuads = 3 * 2 * numRings * numSegments.
 * The vertex colors are interpolated between the west end (red) and the origin (green) and between the origin (green) and the east end (blue).
 * The texture coordinates map the entire image around the cylinder.
 * The vertex normals point out of the cylinder.
 * 
 * @param numRings Number of rings the torus is split into.
 * @param numSegments Number of segments each ring is split into.
 * @param radius1 Radius of the circle around the origin on which the torus lies.
 * @param radius2 Radius of the cylinder on the circle around the origin.
 * @return An IModel representing the torus.
 */
//======================
IModel* IModel::createTorus( int numRings, int numSegments, float radius1, float radius2 )
{
	static const float pi = 4.0f * atan( 1.0f );

	// two triangles per quad, numRings * numSegments quads
	IVertexStream* stream = IVertexStream::create( 6 * numRings * numSegments );

	// access arrays
	vec3_t* v = stream->v();
	vec3_t* n = stream->n();
	vec2_t* t = stream->t();
	vec4_t* c = stream->c();

	vec4_t colorWest( 1,0,0,1 );
	vec4_t colorMid ( 0,1,0,1 );
	vec4_t colorEast( 0,0,1,1 );

	// premultiply color values for interpolation below...
	colorWest = colorWest * ( 1.0f / ( radius1 + radius2 ) );
	colorMid  = colorMid  * ( 1.0f / ( radius1 + radius2 ) );
	colorEast = colorEast * ( 1.0f / ( radius1 + radius2 ) );

	// for each ring
	for( int i = 0 ; i < numRings ; i++ )
	{
		float oldalpha = 2.0f * pi * float(i)   / float(numRings);
		float alpha    = 2.0f * pi * float(i+1) / float(numRings);

		float U1 = 1.0f - float(i)   / float( numRings );
		float U2 = 1.0f - float(i+1) / float( numRings );

		float sa1 = sin( oldalpha );
		float sa2 = sin( alpha );
		float ca1 = cos( oldalpha );
		float ca2 = cos( alpha );

		// for each segment
		for( int j = 0 ; j < numSegments ; j++ )
		{
			vec3_t p,p2;

			float oldbeta = 2.0f * pi * float(j)   / float(numSegments);
			float beta    = 2.0f * pi * float(j+1) / float(numSegments);

			float V1 = ( 1.0f - float(j)   / float(numSegments) ) + 0.5f;
			float V2 = ( 1.0f - float(j+1) / float(numSegments) ) + 0.5f;

			float sb1 = sinf( oldbeta );
			float sb2 = sinf( beta );
			float cb1 = cosf( oldbeta );
			float cb2 = cosf( beta );

#define DEFINE_VERTEX(ring,seg) \
	/* point on unit circle */ \
	p = vec3_t( cb ## seg, sb ## seg, 0.0f ); \
	/* point on circle rotated around Y axis*/ \
	*n = vec3_t( p.x * ca ## ring, p.y, p.x * sa ## ring ); \
	/* translate and scale */ \
	p2 = p * radius2; p2.x += radius1; \
	/* transformed and rotated around Y axis */ \
	*v = vec3_t( p2.x * ca ## ring, p2.y, p2.x * sa ## ring ); \
	/* tex coord */ \
	*t = vec2_t( U ## ring , V ## seg ); \
	/* interpolate color along X axis */ \
	*c = v->x < 0.0f ? \
		( colorWest * (-(v->x)) + colorMid * ( 1 + (v->x)) ) : \
		( colorEast *   (v->x)  + colorMid * ( 1 - (v->x)) ); \
	/* advance pointers */ \
	v++; n++; t++; c++;


			DEFINE_VERTEX( 1,1 );
			DEFINE_VERTEX( 1,2 );
			DEFINE_VERTEX( 2,2 );
			DEFINE_VERTEX( 2,2 );
			DEFINE_VERTEX( 2,1 );
			DEFINE_VERTEX( 1,1 );

#undef DEFINE_VERTEX
		}
	}

	// bounding box is symetric
	vec3_t maxs( radius1 + radius2, radius2, radius1 + radius2 );

	// setup tangent space vectors
	stream->coumputeTangentVectors();

	return new CBaseModel( QString( "Torus" ), GL_TRIANGLES,
		maxs * -1.0f, maxs, radius1+radius2, stream );
}


//=============================================================================
//	single point test model
//=============================================================================

//======================
/** Creates a model that is only a singla point located in the origin.
 * This single point is located in the origin, with the normal (0,0,1),
 * the texture coordinates (0,0) and the color (1,1,1,1).
 * It is intended to be used as input for the geometry shader to make sure
 * it will be executed only once.
 */
//======================
IModel* IModel::createPoint( void )
{
	IVertexStream* stream = IVertexStream::create( 1 );

	*(stream->v()) = vec3_t( 0,0,0 );
	*(stream->n()) = vec3_t( 0,0,1 ); // towards camera position
	*(stream->t()) = vec2_t( 0,0 );
	*(stream->c()) = vec4_t( 1,1,1,1 );

	return new CBaseModel( QString( "Single Point" ), GL_POINTS, 
							vec3_t(0,0,0), vec3_t(0,0,0), 0.0f, stream );
}





