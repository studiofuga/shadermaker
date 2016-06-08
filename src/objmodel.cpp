//=============================================================================
/** @file		objmodel.cpp
 *
 * Implements IMeshModel.
 *
	@internal
	created:	2008-01-25
	last mod:	2008-02-02

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTime>
#include <QMessageBox>
#include <QApplication>

#include "application.h"
#include "model.h"


//=============================================================================
//	CObjModel
//=============================================================================

/** Implementation of IMeshModel.
 */
class CObjModel : public IMeshModel
{
public:
	/** Constructs a CObjModel object. */
	CObjModel( void );
	virtual ~CObjModel( void );

	// IModel interface
	QString getName( void ) { return QString( "Mesh" ); }
	void    render( const VertexAttribLocations* attribs, const vec4_t * overrideColor );
	void	renderNormals( void );
	void	renderTangents( void );
	int		getPrimitiveType( void );
	QString	getPrimitiveTypeName( void );
	float	getBoundingRadius( void );
	void	getBoundingBox( vec3_t & mins, vec3_t & maxs );

	// IMeshModel interface
	bool	loadObjModel( const QString & fileName );

	/** Prints mesh statistics to stderr. */
	void	printStatistics( void ) const;

private:

	/** Helper class for displaying the wait cursor. */
	class AutoWaitCursor
	{
	public:
		AutoWaitCursor( void )
		{
			QApplication::setOverrideCursor( Qt::WaitCursor );
		}

		virtual ~AutoWaitCursor( void )
		{
			QApplication::restoreOverrideCursor();
		}
	};


	/** Helper for indexing the data arrays. */
	class Index
	{
	public:
		Index( int V=0, int N=0, int T=0 )
		{
			v = V; n = N; t = T;
		}

		int v,n,t; // vertex, normal, texcoord
	};

	/** Face information. */
	class Face
	{
	public:
		Face( int StartIndex=0, int NumIndices=0 )
		{
			startIndex = StartIndex;
			numIndices = NumIndices;
		}

		int startIndex, numIndices;
	};


	// misc helpers
	void    clearContent( void );

	// parsing
	QString loadObjFile( const QString & fileName );
	void    countEntitiesInObj( const QStringList & obj );
	void	parseEntities( const QStringList & obj );
	vec3_t	parseVec3( const QStringList & lineWithKeyWord );
	vec2_t	parseVec2( const QStringList & lineWithKeyWord );
	Index	parseIndex( const QString & indices );

	// data post processing
	void	computeBoundingVolumes( void );
	void	computeNormals( void );
	void	computeTexCoords( void );
	void	computeTangents( void );
	void	rescaleModel( void );

	// display lists
	void	setupDisplayListModel( const VertexAttribLocations & attribs, bool colored );
	void	setupDisplayListNormals( void );
	void	setupDisplayListTangents( void );

	// data array sizes
	int		m_numVertices;
	int		m_numNormals;
	int		m_numTexCoords;
	int		m_numFaces;
	int		m_numIndices;

	// data arrays
	vec3_t*	m_vertices;		// [ m_numVertices ]
	vec3_t* m_normals;		// [ m_numNormals ]
	vec2_t* m_texCoords;	// [ m_numTexCoords ]
	Face*	m_faces;		// [ m_numFaces ]
	Index*	m_indices;		// [ m_numIndices ]
	vec3_t*	m_tangents;		// [ m_numIndices ]
	vec3_t*	m_bitangents;	// [ m_numIndices ]

	// rendering acceleration
	GLuint	m_displayLists; // +0: model uncolored, +1: model colored, +2: normals, +3: tangents
	VertexAttribLocations m_attribsInDL; // values encoded into the display list

	// metadata
	int		m_primitiveType; // every mesh must have this type!

	int		m_loadTime; // in milliseconds
	QString	m_fileName;

	// bounding volumes
	float	m_boundingRadius;
	vec3_t	m_mins, m_maxs;
};


//======================
/** Creates a IMeshModel object.
 */
//======================
IMeshModel* IMeshModel::createMeshModel( void )
{
	return new CObjModel();
}


/*
========================
constructor
========================
*/
CObjModel::CObjModel( void )
{
	m_displayLists = 0;
	m_primitiveType = GL_POINTS;
	m_boundingRadius = 0.0f;
	m_mins = m_maxs = vec3_t( 0,0,0 );

	m_fileName = QString( "" );
	m_loadTime = 0;

	m_numVertices = 0;
	m_numNormals = 0;
	m_numTexCoords = 0;
	m_numFaces = 0;
	m_numIndices = 0;

	m_vertices = NULL;
	m_normals = NULL;
	m_texCoords = NULL;
	m_faces = NULL;
	m_indices = NULL;
	m_tangents = NULL;
	m_bitangents = NULL;
}


/*
========================
Destructor
========================
*/
CObjModel::~CObjModel( void )
{
	clearContent();
}


/*
========================
loadObjModel
========================
*/
bool CObjModel::loadObjModel( const QString & fileName )
{
	// use a wait cursor that pops itself from the stack.
	AutoWaitCursor waitamoment;

	// remove content like specification sais.
	clearContent();

	// save load time
	m_loadTime = 0;
	QTime time;
	time.start();

	//
	// get the object file as a string.
	//
	QString objFileRaw = loadObjFile( fileName );
	if( objFileRaw.isEmpty() )
		return false;

	// split into lines for easier parsing
	QStringList objFile = objFileRaw.split( QRegExp( "\\n" ), QString::SkipEmptyParts );

	// setup m_num??? vars
	countEntitiesInObj( objFile );

	// no data available? then nothing to do..
	if( m_numVertices == 0 ||
		m_numIndices == 0 )
	{
		return false;
	}

	// setup data arrays
	m_vertices  = new vec3_t[ m_numVertices ];
	m_normals   = new vec3_t[ m_numNormals ];
	m_texCoords = new vec2_t[ m_numTexCoords ];
	m_faces		= new Face  [ m_numFaces ];
	m_indices	= new Index	[ m_numIndices ];
	m_tangents	= new vec3_t[ m_numIndices ];
	m_bitangents= new vec3_t[ m_numIndices ];

	// load data into the vertex arrays
	parseEntities( objFile );

	// post process data
	rescaleModel();
	computeBoundingVolumes();

	// auto-create missing stuff
	if( m_numNormals == 0 )   { computeNormals(); }
	if( m_numTexCoords == 0 ) { computeTexCoords(); }

	// .OBJ files don't support tangent/bitangent
	// -> create them now.
	computeTangents();

	// display lists
	m_displayLists = glGenLists( 4 );
	setupDisplayListModel( m_attribsInDL, false );
	setupDisplayListModel( m_attribsInDL, true );
	setupDisplayListNormals();
	setupDisplayListTangents();

	// report triangles
	m_primitiveType = GL_TRIANGLES;

	m_loadTime = time.elapsed();
	m_fileName = extractFileNameFromPath( fileName );

	printStatistics();
	return true;
}


/*
========================
clearContent
========================
*/
void CObjModel::clearContent( void )
{
	// free display lists
	if( m_displayLists != 0 )
	{
		glDeleteLists( m_displayLists, 3 );
		m_displayLists = 0;
	}

	SAFE_DELETE_ARRAY( m_vertices );
	SAFE_DELETE_ARRAY( m_normals );
	SAFE_DELETE_ARRAY( m_texCoords );
	SAFE_DELETE_ARRAY( m_faces );
	SAFE_DELETE_ARRAY( m_indices );
	SAFE_DELETE_ARRAY( m_tangents );
	SAFE_DELETE_ARRAY( m_bitangents );

	m_numVertices = 0;
	m_numNormals = 0;
	m_numTexCoords = 0;
	m_numFaces = 0;
	m_numIndices = 0;

	m_loadTime = 0;

	m_primitiveType = GL_POINTS;

	m_fileName = QString( "" );
}


/*
========================
render
========================
*/
void CObjModel::render( const VertexAttribLocations* attribs, const vec4_t * overrideColor )
{
	// not initialized / loaded
	if( m_displayLists == 0 )
		return;

	// rebuild the display lists when the attrib locations change
	if( attribs != NULL )
	{
		// Make sure we don't rebuild the DL if the GLSL program is deactivated!
		// Only rebuild it, when the actual LOCATIONS changes!
		if( ( attribs->tangent != -1 &&
			  attribs->tangent != m_attribsInDL.tangent ) ||
			( attribs->bitangent != -1 &&
			  attribs->bitangent != m_attribsInDL.bitangent ) )
		{
		//	fprintf( stderr, "rebuilding DL...\n" );
			setupDisplayListModel( *attribs, true );
			setupDisplayListModel( *attribs, false );
		}
	}

	// call the list with/without colors
	if( overrideColor != NULL )
	{
		glColor4fv( overrideColor->toFloatPointer() );
		glCallList( m_displayLists );
	}

	else
	{
		glCallList( m_displayLists + 1 );
	}
}


/*
========================
renderNormals
========================
*/
void CObjModel::renderNormals( void )
{
	if( m_displayLists != 0 )
	{
		glCallList( m_displayLists + 2 );
	}
}


/*
========================
renderTangents
========================
*/
void CObjModel::renderTangents( void )
{
	if( m_displayLists != 0 )
	{
		glCallList( m_displayLists + 3 );
	}
}


/*
========================
setupDisplayListModel
========================
*/
void CObjModel::setupDisplayListModel( const VertexAttribLocations & attribs, bool colored )
{
	// save for later...
	m_attribsInDL = attribs;

	glNewList( m_displayLists + ( colored?1:0 ), GL_COMPILE );

	//
	// for each face...
	//
	for( int i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		glBegin( GL_TRIANGLE_FAN );

		//
		// for each index...
		//
		for( int j = 0 ; j < f.numIndices ; j++ )
		{
			const Index & idx = m_indices[ f.startIndex + j ];
			const vec3_t & v = m_vertices[ idx.v ];

			// optional attributes
			if( attribs.tangent != -1 )
			{
				glVertexAttrib3fv( attribs.tangent, m_tangents[ f.startIndex + j ].toFloatPointer() );
			}
			if( attribs.bitangent != -1 )
			{
				glVertexAttrib3fv( attribs.bitangent, m_bitangents[ f.startIndex + j ].toFloatPointer() );
			}

			// 'must' attribute
			if( colored )
			{
				// we are in the unit cube...
				vec3_t color = vec3_t( 1.0f, 1.0, 1.0f ) - v.absolute();
				glColor3fv( color.toFloatPointer() );
			}

			glTexCoord2fv( m_texCoords[ idx.t ].toFloatPointer() );
			glNormal3fv  ( m_normals[ idx.n ].toFloatPointer() );
			glVertex3fv  ( v.toFloatPointer() );
		}

		glEnd();
	}

	glEndList();
}


/*
========================
setupDisplayListNormals
========================
*/
void CObjModel::setupDisplayListNormals( void )
{
	if( m_numNormals <= 0 )
		return;

	glNewList( m_displayLists + 2, GL_COMPILE );
	glBegin( GL_LINES );

	//
	// for each face
	//
	for( int i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		// for each index
		for( int j = 0 ; j < f.numIndices ; j++ )
		{
			const Index & idx = m_indices[ f.startIndex + j ];
			const vec3_t & v = m_vertices[ idx.v ];
			const vec3_t & n = m_normals [ idx.n ];

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

			glVertex3fv( v.toFloatPointer() );
			glVertex3fv( ( v + n * 0.3f ).toFloatPointer() );
		}
	}

	glEnd();
	glEndList();
}


/*
========================
setupDisplayListTangens
========================
*/
void CObjModel::setupDisplayListTangents( void )
{
	int i,j;
	float length = 0.1f;

	glNewList( m_displayLists + 3, GL_COMPILE );
	glBegin( GL_LINES );

	for( i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		for( j = 0 ; j < f.numIndices ; j++ )
		{
			int k = f.startIndex + j;
			const Index & idx = m_indices[ k ];

			// tangent
			glColor3f( 1,0,0 );
			glVertex3fv( m_vertices[ idx.v ].toFloatPointer() );
			glVertex3fv( ( m_vertices[ idx.v ] + m_tangents[ k ] * length ).toFloatPointer() );

			// bitangent
			glColor3f( 0,1,0 );
			glVertex3fv( m_vertices[ idx.v ].toFloatPointer() );
			glVertex3fv( ( m_vertices[ idx.v ] + m_bitangents[ k ] * length ).toFloatPointer() );

			// normal
			glColor3f( 0,0,1 );
			glVertex3fv( m_vertices[ idx.v ].toFloatPointer() );
			glVertex3fv( ( m_vertices[ idx.v ] + m_normals[ idx.n ] * length ).toFloatPointer() );
		}
	}

	glEnd();
	glEndList();
}


/*
========================
parseEntities

 reads vertex/triangle data from the object and places it into the data arrays.
 @param obj String representation of the .OBJ file split into single lines.
 @pre Assumes data arrays are allocated.
========================
*/
void CObjModel::parseEntities( const QStringList & obj )
{
	// array offsets
	int numVertices = 0;
	int numNormals = 0;
	int numTexCoords = 0;
	int numFaces = 0;
	int numIndices = 0;

	// loop through all lines
	for( int i = 0 ; i < obj.size() ; i++ )
	{
		// spilt the line into tokens.
		const QStringList & tokens = 
			obj.at( i ).split( QRegExp( "\\s+" ), QString::SkipEmptyParts );

		// malformatted...
		if( tokens.size() < 1 )
			continue;

		const QString & str = tokens.at( 0 );

		// vertex
		if( str == QString( "v" ) && numVertices < m_numVertices )
		{
			m_vertices[ numVertices++ ] = parseVec3( tokens );
		}

		// normal
		else if( str == QString( "vn" ) && numNormals < m_numNormals )
		{
			m_normals[ numNormals++ ] = parseVec3( tokens );
		}

		// tex coords
		else if( str == QString( "vt" ) && numTexCoords < m_numTexCoords )
		{
			m_texCoords[ numTexCoords++ ] = parseVec2( tokens );
		}

		// face
		else if( str == QString( "f" ) && numFaces < m_numFaces )
		{
			int n = tokens.size() - 1;

			if( numIndices + n <= m_numIndices )
			{
				m_faces[ numFaces++ ] = Face( numIndices, n );

				for( int j = 0 ; j < n ; j++ )
				{
					m_indices[ numIndices++ ] = parseIndex( tokens.at(j+1) );
				}
			}
		}
	}
}


/*
========================
countEntitiesInObj

 counts vertices, normals, tex coords and triangles in a .OBJ file.
 @param obj String representation of the .OBJ file split into single lines.
========================
*/
void CObjModel::countEntitiesInObj( const QStringList & obj )
{
	// skan each line of the object
	for( int i = 0 ; i < obj.size() ; i++ )
	{
		// tokenize...
		const QString & line = obj.at( i );
		if( line.isEmpty() )
			continue;

		QStringList split = line.split( QRegExp("\\s+"), QString::SkipEmptyParts );
		if( split.size() < 1 ) // malformatted...
			continue;

		const QString & first = split.at( 0 );

		// vertex position
		if( first == QString( "v" ) )
		{
			m_numVertices++;
		}

		// vertex normal
		else if( first == QString( "vn" ) )
		{
			m_numNormals++;
		}

		// vertex tex coord
		else if( first == QString( "vt" ) )
		{
			m_numTexCoords++;
		}

		// face
		else if( first == QString( "f"  ) )
		{
			m_numFaces++;
			m_numIndices += split.size() - 1;
		}
	}
}


/*
========================
parseIndex

 takes an index string of one of these formats:
 - "V/T/N"  or "V//N"
 - "V/T"
 - "V"
========================
*/
CObjModel::Index CObjModel::parseIndex( const QString & indices )
{
	Index idx;
	QStringList list = indices.split( QRegExp("/"), QString::KeepEmptyParts );

	switch( list.size() )
	{
	// "V"
	case 1:	idx = Index( list.at(0).toInt()-1 ); break;

	// "V/T"
	case 2: idx = Index( list.at(0).toInt()-1, 0, list.at(1).toInt() ); break;

	// "V/T/N"
	case 3: idx = Index( list.at(0).toInt()-1, list.at(2).toInt()-1, list.at(1).toInt()-1 ); break;
	}

	// do range check
	if( idx.v >= m_numVertices  ) { idx.v = 0; }
	if( idx.n >= m_numNormals   ) { idx.n = 0; }
	if( idx.t >= m_numTexCoords ) { idx.t = 0; }

	return idx;
}


/*
========================
parseVec3

 parses a string list of format "bla %f %f %f" into a 3D vector.
 missing tokens are set to zero.

 @param line OBJ text line including the leading line keyword (v,vn,vt,...)
========================
*/
vec3_t CObjModel::parseVec3( const QStringList & line )
{
	int size = line.size();

	float f1 = ( size > 1 ) ? line.at(1).toFloat() : 0.0f;
	float f2 = ( size > 2 ) ? line.at(2).toFloat() : 0.0f;
	float f3 = ( size > 3 ) ? line.at(3).toFloat() : 0.0f;

	return vec3_t( f1,f2,f3 );
}


/*
========================
parseVec2

 @see parseVec3
========================
*/
vec2_t CObjModel::parseVec2( const QStringList & line )
{
	int size = line.size();

	float f1 = size > 1 ? line.at(1).toFloat() : 0.0f;
	float f2 = size > 2 ? line.at(2).toFloat() : 0.0f;

	return vec2_t( f1,f2 );
}


/*
========================
loadObjFile
========================
*/
QString CObjModel::loadObjFile( const QString & fileName )
{
	QFile file( fileName );

	if( !file.open( QFile::ReadOnly | QFile::Text ) )
	{
		QMessageBox::warning( NULL,
							  QString( CONFIG_STRING_ERRORDLG_TITLE ),
							  QString( "Cannot read file %1:\n%2." )
							  .arg(fileName)
							  .arg(file.errorString()));
		return QString( "" );
	}

	QTextStream in( &file );
	return in.readAll();
}


/*
========================
rescaleModel

 Rescales vertex positions that the model fits into the unit cube.
 The vertex positions are ofsetted that the model center lies in the origin.
========================
*/
void CObjModel::rescaleModel( void )
{
	int i;
	float s = -1.0f;
	vec3_t mins( +99999, +99999, +99999 );
	vec3_t maxs( -99999, -99999, -99999 );

	// find bounding box
	for( i = 0 ; i < m_numVertices ; i++ )
	{
		const vec3_t & v = m_vertices[ i ];
		if( mins.x > v.x ) { mins.x = v.x; }
		if( mins.y > v.y ) { mins.y = v.y; }
		if( mins.z > v.z ) { mins.z = v.z; }
		if( maxs.x < v.x ) { maxs.x = v.x; }
		if( maxs.y < v.y ) { maxs.y = v.y; }
		if( maxs.z < v.z ) { maxs.z = v.z; }
	}

	// set origin to the model center by offsetting the position vectors.
	vec3_t ofs = ( maxs + mins ) * 0.5f;
	for( i = 0 ; i < m_numVertices ; i++ )
	{
		m_vertices[ i ] = m_vertices[ i ] - ofs;

		float maxi = m_vertices[ i ].absoluteCoordMaximum();
		if( s < maxi ) { s = maxi; }
	}

	// avoid division inside loop
	if( s != 0.0f ) {
		s = 1.0f / s;
	}

	// scale vertices
	for( i = 0 ; i < m_numVertices ; i++ )
	{
		m_vertices[ i ] = m_vertices[ i ] * s;
	}
}


/*
========================
computeBoundingVolumes
========================
*/
void CObjModel::computeBoundingVolumes( void )
{
	float radiusSq = 0.0f;
	m_mins = vec3_t( +999999, +999999, +999999 );
	m_maxs = vec3_t( -999999, -999999, -999999 );

	for( int i = 0 ; i < m_numVertices ; i++ )
	{
		const vec3_t & v = m_vertices[ i ];

		// bounding radius
		if( radiusSq < v.lengthSq() )
			radiusSq = v.lengthSq();

		// bounding box
		if( m_mins.x > v.x ) { m_mins.x = v.x; }
		if( m_mins.y > v.y ) { m_mins.y = v.y; }
		if( m_mins.z > v.z ) { m_mins.z = v.z; }
		if( m_maxs.x < v.x ) { m_maxs.x = v.x; }
		if( m_maxs.y < v.y ) { m_maxs.y = v.y; }
		if( m_maxs.z < v.z ) { m_maxs.z = v.z; }
	}

	m_boundingRadius = sqrt( radiusSq );
}


/*
========================
computeNormals

 Assigns one normal to every vertex.
========================
*/
void CObjModel::computeNormals( void )
{
	int i,j;

	// realloc normal array
	SAFE_DELETE_ARRAY( m_normals );
	m_numNormals = m_numVertices;
	m_normals = new vec3_t[ m_numNormals ]; // assumes all normals are Null-vectors!

	// compute face normals
	for( i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		// degenerated face...
		if( f.numIndices < 3 )
			continue;

		// compute face normal
		const vec3_t & p0 = m_vertices[ m_indices[ f.startIndex + 0 ].v ];
		const vec3_t & p1 = m_vertices[ m_indices[ f.startIndex + 1 ].v ];
		const vec3_t & p2 = m_vertices[ m_indices[ f.startIndex + 2 ].v ];
		vec3_t n = ( p1 - p0 ).crossProduct( p2 - p0 );

		for( j = 0 ; j < f.numIndices ; j++ )
		{
			Index & idx = m_indices[ f.startIndex + j ];

			// each vertex has exactly one normal.
			idx.n = idx.v;

			// add normals to the vertex.
			m_normals[ idx.n ] = m_normals[ idx.n ] + n;
		}
	}

	// normalize the normals
	// -> this should 'smooth' the normal on shared vertices.
	for( i = 0 ; i < m_numNormals ; i++ )
	{
		m_normals[ i ] = m_normals[ i ].normalize();
	}
}


/*
========================
computeTexCoords

 Assigns one texture coord st-pair to every vertex.
========================
*/
void CObjModel::computeTexCoords( void )
{
	int i,j;

	// realloc tex coord array
	SAFE_DELETE_ARRAY( m_texCoords );
	m_numTexCoords = m_numVertices;
	m_texCoords = new vec2_t[ m_numTexCoords ];

	// compute coords
	for( i = 0 ; i < m_numTexCoords ; i++ )
	{
		const vec3_t & v = m_vertices[ i ];

		// do sphere mapping...
		float invroot = sqrt( v.x*v.x + v.y*v.y + ( v.z+1 )*( v.z+1 ) );
		if( invroot != 0.0f ) { invroot = 1.0f / invroot; }
		m_texCoords[ i ] = vec2_t( (invroot*v.x)+1, (invroot*v.y+1) ) * 0.5f;
	}


	// assign indices
	for( i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		for( j = 0 ; j < f.numIndices ; j++ )
		{
			Index & idx = m_indices[ f.startIndex + j ];
			idx.t = idx.v; // one tex coord pair for every vertex.
		}
	}
}


/*
========================
computeTangents

 Assigns one pair of tangent vectors to every INDEX.
 Data arrrays are already allocated.
 Assumes the tangent data arrays are all zeroed-out.
========================
*/
void CObjModel::computeTangents( void )
{
	int i,j;

	// compute tangent vectors for every face.
	for( i = 0 ; i < m_numFaces ; i++ )
	{
		const Face & f = m_faces[ i ];

		// degenerated face
		if( f.numIndices < 3 )
			continue;

		const Index & idx0 = m_indices[ f.startIndex + 0 ];
		const Index & idx1 = m_indices[ f.startIndex + 1 ];
		const Index & idx2 = m_indices[ f.startIndex + 2 ];

		vec3_t e1 = m_vertices [ idx1.v ] - m_vertices [ idx0.v ];
		vec3_t e2 = m_vertices [ idx2.v ] - m_vertices [ idx0.v ];
		vec2_t t1 = m_texCoords[ idx1.t ] - m_texCoords[ idx0.t ];
		vec2_t t2 = m_texCoords[ idx2.t ] - m_texCoords[ idx0.t ];

		float length = t1.y * t2.x - t1.x *t2.y;
		if( fabs(length) > 0.000001 ) // triangle not degenerated
		{
			// solve linear equations
			vec3_t planeTangent = ( e2 * t1.y - e1 * t2.y );

			// adjust tangents to the vertex normal
			for( j = 0 ; j < f.numIndices ; j++ )
			{
				// assumed to be normalized.
				const vec3_t & normal = m_normals[ m_indices[ f.startIndex + j ].n ];

				// orthogonalize
				vec3_t tangent = planeTangent - normal * planeTangent.dotProduct( normal );
				tangent = tangent.normalize();

				m_tangents  [ f.startIndex + j ] = tangent;
				m_bitangents[ f.startIndex + j ] = tangent.crossProduct( normal );
			}
		}
	}
}


/*
========================
printStatistics
========================
*/
void CObjModel::printStatistics( void ) const
{
	fprintf( stderr, "Statistics for .OBJ model '%s'\n",
        (const char*)m_fileName.toStdString().c_str() );

	fprintf( stderr,
		"vertices:    %d\n"
		"normals:     %d\n"
		"tex coords:  %d\n"
		"faces:       %d\n"
		"indices:     %d\n",
		m_numVertices,
		m_numNormals,
		m_numTexCoords,
		m_numFaces,
		m_numIndices );

	int memory =
		m_numVertices  * sizeof(vec3_t) +
		m_numNormals   * sizeof(vec3_t) +
		m_numTexCoords * sizeof(vec2_t) +
		m_numFaces     * sizeof(Face) +
		m_numIndices   * sizeof(Index) +
		m_numIndices   * ( sizeof(vec3_t) + sizeof(vec3_t) ); // tangent+bitangent
	fprintf( stderr, "memory required: %d Byte == %d kByte\n", memory, memory/1024 );

	fprintf( stderr, "bounding radius: %f\n", m_boundingRadius );
	fprintf( stderr, "mins/maxs: ( %f %f %f ),  ( %f %f %f )\n",
		m_mins.x, m_mins.y, m_mins.z, m_maxs.x, m_maxs.y, m_maxs.z );

	fprintf( stderr, "load time: %d ms\n", m_loadTime );
}


/*
========================
getPrimitiveType
========================
*/
int CObjModel::getPrimitiveType( void )
{
	return m_primitiveType;
}


/*
========================
getPrimitiveTypeName
========================
*/
QString CObjModel::getPrimitiveTypeName( void )
{
	return primitiveTypeName( m_primitiveType );
}


/*
========================
getBoundingRadius
========================
*/
float CObjModel::getBoundingRadius( void )
{
	return m_boundingRadius;
}


/*
========================
getBoundingBox
========================
*/
void CObjModel::getBoundingBox( vec3_t & mins, vec3_t & maxs )
{
	mins = m_mins;
	maxs = m_maxs;
}

