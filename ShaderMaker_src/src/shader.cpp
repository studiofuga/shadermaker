//=============================================================================
/** @file		shader.cpp
 *
 * Implements the IShader interface.
 *
	@internal
	created:	2007-10-24
	last mod:	2008-02-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QtCore/QTime>
#include <QMessageBox>
#include "application.h"
#include "shader.h"

#include <assert.h>


//=============================================================================
//	Geometry shader man page
//=============================================================================

/** @page geometryshader Geometry shaders
This tutorial explains how to use geometry shaders.

Geometry shaders are still quite new ( Feb. 2008 ), so your OpenGL implementation
might not support this feature. If it does, you can see a third shader type in the
editor window, the geometry shader. This shader type is not attached to the program
by default, you have to check the check-box first. If you compile and run the initial shader source,
you can see the test model without any modifications, because the geometry shader passes
through all its input.

Geometry shaders have access to connectivity information of vertices.
They can read the vertex attributes of all vertices used in a single primitive.
That's the difference between the vertex and the geometry shader, because vertex
shader only 'see' one individual vertex.

There is one thing that makes geometry shader more complicated than vertex and fragment shaders.
Geometry shaders need to know the primitive type they are processing.
And they need to know  it at link time!
Go to the 'Scene' tab and look at the Geometry Shader group box.
There are two values, the input primitive type and the output primitive type.
The input primitive type depends on the test model and is almost always GL_TRIANGLES;
The output primitive type is what you want your geometry shader to output.
It is GL_TRIANGLE_STRIP in most cases.
You can change this value to GL_LINE_STRIP for example.
If you change it, you have to relink your program.
The program then keeps this value, until you relink it again.
The input primitive type depends on the selected test model.
So you must choose the test model BEFORE linking the shader.
If you change the test model and thereby change the input primitive type,
you must relink the program.

Now let's modify the initial geometry shader.
Reverse the order in which the vertices define the primitive.
To do that, replace the access index i with

gl_VerticesIn - i - 1

Now you should see the test model like before. So what is it good for?
Choose the cube test model and check the 'Back Face Culling' check box.
Because the order of the vertices is reversed, front faces become back faces and vice versa.
If you apply a texture map to the cube, you can see the effect even better.

There is a test model called 'Single Point'. And this is really just a single point.
Now you have a scenario to use this test model:
The geometry shader is executed for every input primitive and with only
a single point as input, the geometry shader is executed exactly once!
This allows you to create your own geometry on the GPU without overdrawing it several times.

*/


//=============================================================================
//	IShader implementation
//=============================================================================

/** Implementation of IShader.
 * This class doensn't add new functionality. It is only used to hide
 * the class definition from the interface definition.
 */
class CShader : public IShader
{
public:
	/** Constructs a shader object. */
	CShader( void );
	virtual ~CShader( void ); ///< Destructor.

	// initialization
	bool init( void );
	void shutdown( void );

	// rendering
	bool bindState( VertexAttribLocations & attribs );

	// shader state
	void setShaderSource( int shaderType, const QString & source );
	void setGeometryInputType( int type );
	void setGeometryOutputType( int type );
    void setGeometryOutputNum( int type );

	// linking
	bool compileAndLink( void );
	void deactivateProgram( void );
	QString getBuildLog( void );

	// type management
	bool isShaderTypeAvailable( int type );

	// IUniformState
	int getActiveUniforms( void );
	CUniform getUniform( int index );
	void setUniform( int index, const CUniform & u );

private:

	// Does actual compile and link work.
	// Is is encapsulated into a try/catch block for driver exceptions!
	bool compileAndLink2( void );
	bool compileAndAttachShader( int shaderType );
	bool linkAndValidateProgram( void );
	void setupProgramParameters( void );

	// logs additional linking info
	void logActiveAttributes( void );
	void logActiveUniforms( void );

	// sets initial uniform state after linking
	void setupInitialUniforms( void );
	void setupRememberedUniformState( void );

	// checks wether this is the time variable and updates it.
	void updateTimeVariable( CUniform & u );

	// shader type symbol mapping
	int toGlShaderType( int symbol );

	// state of the uniforms
	QVector< CUniform > m_activeUniforms;

	// from last state
	// -> only updated if the program was successfully linked!
	QVector< CUniform > m_oldUniforms;

	// vertex and fragment shaders are included in OpenGL 2.0
	// -> geometry shader is still an extension
	bool m_geometryShaderAvailable;

	// wether the program is ready to operate
	bool m_linked;

	// results of compile and link operation
	QString m_log;

	// timer for the "uniform float time"
	QTime m_timer;

	// program parameters
    int m_num_output;
	int m_geometryInputType; // for geometry shader
	int m_geometryOutputType;

	// where vertex arrays can write to...
	VertexAttribLocations m_attribLocations;

	// source code for each shader type.
	QString m_shaderSources[ MAX_SHADER_TYPES ];

	// objects
	GLuint	m_shaders[ MAX_SHADER_TYPES ];
	GLuint	m_program;
};


// construction
CShader::CShader( void )
{
	m_geometryShaderAvailable = false;
	m_program = 0;
	m_linked = false;

    m_num_output = 4;
    m_geometryInputType  = GL_LINE_STRIP_ADJACENCY;
	m_geometryOutputType = GL_TRIANGLE_STRIP;

	for( int i = 0 ; i < MAX_SHADER_TYPES ; i++ )
		m_shaders[ i ] = 0;
}

CShader::~CShader( void )
{
}


/*
========================
create
========================
*/
IShader* IShader::create( void )
{
	return new CShader();
}


/*
========================
init

 assumes OpenGL context is ready
========================
*/
bool CShader::init( void )
{
	// clean up old state.
	shutdown();

	//
	// find out wether we can use geometry shaders
	//
#ifdef CONFIG_ENABLE_GEOMETRY_SHADER
	const char* ext = (const char*)glGetString( GL_EXTENSIONS );
	if( strstr( ext, "GL_EXT_geometry_shader4" ) != NULL )
	{
		m_geometryShaderAvailable = true;
	}
#endif // CONFIG_ENABLE_GEOMETRY_SHADER

	return true;
}


/*
========================
shutdown
========================
*/
void CShader::shutdown( void )
{
	m_geometryShaderAvailable = false;

	deactivateProgram();

	for( int i = 0 ; i < MAX_SHADER_TYPES ; i++ )
		m_shaderSources[ i ] = QString( "" );
}


/*
========================
deactivateProgram

 Destroy all shaders and program. They are re-created when needed.
 Does not touch the shader source.
========================
*/
void CShader::deactivateProgram( void )
{
	// make sure we have no references to the program.
	m_linked = false; // in case of empty sources
	m_attribLocations = VertexAttribLocations();

	glUseProgram( 0 );

	// delete program object
	if( m_program != 0 )
	{
		glDeleteProgram( m_program );
		m_program = 0;
	}

	// delete shader objects
	for( int i = 0 ; i < MAX_SHADER_TYPES ; i++ )
	{
		if( m_shaders[ i ] != 0 )
		{
			glDeleteShader( m_shaders[ i ] );
			m_shaders[ i ] = 0;
		}
	}
}


/*
========================
compileAndLink
========================
*/
bool CShader::compileAndLink( void )
{
	bool result = false;

	// if we get any exceptions, they must be from the GL driver!
	try
	{
		result = compileAndLink2();
	}
	catch( ... )
	{
		result = false;
		m_log = QString(
			"*** CRITICAL ERROR ***\n"
			"\n"
			"  There was an exception thrown by the OpenGL driver!\n"
			"  You should immediately restart the editor!\n"
			"\n"
			"Check your sources for things like unresolved symbols.\n"
			"Missing varying variables can cause trouble too.\n"
			"\n"
			"Example:\n"
			"\n"
			"varying vec3 notDefinedInVertexShader;\n"
			"float foo(); // nowhere implemented\n"
			"\n"
			"vec3 bar()\n"
			"{\n"
			"    return notDefinedInVertexShader * foo();\n"
			"}\n"
			"\n" );
	}

	return result;
}


/*
========================
compileAndLink2
========================
*/
bool CShader::compileAndLink2( void )
{
	bool totalResult = true;

	// clean up old state
	deactivateProgram();
	m_log = QString();

	// create a new program object
	m_program = glCreateProgram();
	if( m_program == 0 )
	{
		m_log += QString( "ERROR: Failed on glCreateProgram()\n" );
		return false;
	}

	// update shaders
	for( int i = 0 ; i < MAX_SHADER_TYPES ; i++ )
	{
		bool result = compileAndAttachShader( i );
		totalResult = totalResult && result;
	}

	// if we have shaders attached, link them to a program.
	if( totalResult )
	{
		bool result = linkAndValidateProgram();
		totalResult = totalResult && result;
	}

	// post processing...
	if( totalResult )
	{
		setupInitialUniforms();
		setupRememberedUniformState();
	}

	return totalResult;
}


/*
========================
compileAndAttachShader

 assumes the shader is available.
========================
*/
bool CShader::compileAndAttachShader( int shaderType )
{
	GLint status;
	char text[ 4096 ];

	// must be deleted before use!
	assert( m_shaders[ shaderType ] == 0 );

	// shader type not supported, skip
	if( !isShaderTypeAvailable( shaderType ) )
		return true;

	// check if we have source code defined for this shader
	if( m_shaderSources[ shaderType ].isEmpty() )
		return true; // shader not used.

	m_log += QString( "Compiling %1\n" ).arg( getShaderTypeName(shaderType) );

	// create the shader object.
	m_shaders[ shaderType ] = glCreateShader( toGlShaderType( shaderType ) );
	if( m_shaders[ shaderType ] == 0 )
	{
		m_log += QString( "ERROR: failed on glCreateShader( %1 )\n" ).
							arg( getShaderTypeName( shaderType ) );
		return false;
	}

	// recompile shader
	int length = m_shaderSources[ shaderType ].length() + 1;
	char* buf = new char[ length ];
	const GLchar* src = buf;
    strncpy( buf, m_shaderSources[ shaderType ].toStdString().c_str(), length );
	glShaderSource ( m_shaders[ shaderType ], 1, &src, NULL );
	glCompileShader( m_shaders[ shaderType ] );
	delete [] buf;

	// read the log
	memset( text, 0, sizeof(text) );
	glGetShaderInfoLog( m_shaders[ shaderType ], sizeof(text), NULL, text );
	text[ sizeof(text)-1 ] = '\0';
	m_log += QString( "%1\n" ).arg( text );

	// attach/detach
	glGetShaderiv( m_shaders[ shaderType ], GL_COMPILE_STATUS, &status );
	if( status != GL_FALSE )
	{
		// success
		glAttachShader( m_program, m_shaders[ shaderType ] );
		return true;
	}

	return false;
}


/*
========================
linkAndValidateProgram
========================
*/
bool CShader::linkAndValidateProgram( void )
{
	GLint status;
	char text[ 4096 ];

	m_log += QString( "Linking...\n" );

	// things that must be done before linking
	setupProgramParameters();

	glLinkProgram( m_program );

	// read log
	memset( text, 0, sizeof(text) );
	glGetProgramInfoLog( m_program, sizeof(text), NULL, text );
	text[ sizeof(text)-1 ] = '\0';
	m_log += QString( "%1\n\n" ).arg( text );

	// check link status
	glGetProgramiv( m_program, GL_LINK_STATUS, &status );
	m_linked = ( status != GL_FALSE );

	// validate program
	glValidateProgram( m_program );
	glGetProgramiv( m_program, GL_VALIDATE_STATUS, &status );
	m_log += QString( "Validation: %1\n" ).arg( status ? "succeeded" : "failed" );

	// validation log
	memset( text, 0, sizeof(text) );
	glGetProgramInfoLog( m_program, sizeof(text), NULL, text );
	text[ sizeof(text)-1 ] = '\0';
	m_log += QString( "%1\n" ).arg( text );
	m_log += QString( "\n" );

	// restart timer
	m_timer.start();

	// query named attrib locations
	m_attribLocations.tangent   = glGetAttribLocation( m_program, "attrTangent" );
	m_attribLocations.bitangent = glGetAttribLocation( m_program, "attrBitangent" );

	// log log log
	logActiveUniforms();
	logActiveAttributes();

	return m_linked;
}


/*
========================
setupProgramParameters
========================
*/
void CShader::setupProgramParameters( void )
{
	GLint n;

	//
	// geometry shader specific program parameters
	//
	if( m_geometryShaderAvailable )
	{
		// query maximum
        glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &n );

		// set maximum. may be inefficient, but of universal use.
        glProgramParameteriEXT( m_program, GL_GEOMETRY_VERTICES_OUT_EXT, m_num_output );

		// set primitive types
		glProgramParameteriEXT( m_program, GL_GEOMETRY_INPUT_TYPE_EXT,  m_geometryInputType );
		glProgramParameteriEXT( m_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, m_geometryOutputType );
	}
}


/*
========================
setShaderSource
========================
*/
void CShader::setShaderSource( int shaderType, const QString & src )
{
	if( shaderType >= 0 && shaderType < MAX_SHADER_TYPES )
	{
		m_shaderSources[ shaderType ] = src;
	}
}


/*
========================
setGeometryInputType
========================
*/
void CShader::setGeometryInputType( int type )
{
	switch( type )
	{
	case GL_POINTS:
	case GL_LINES:
	case GL_LINES_ADJACENCY_EXT:
	case GL_TRIANGLES:
	case GL_TRIANGLES_ADJACENCY_EXT:
		m_geometryInputType = type;
		break;
	}
}


/*
========================
setGeometryOutputType
========================
*/
void CShader::setGeometryOutputType( int type )
{
	switch( type )
	{
	case GL_POINTS:
	case GL_LINE_STRIP:
	case GL_TRIANGLE_STRIP:
		m_geometryOutputType = type;
		break;
    }
}

void CShader::setGeometryOutputNum(int type)
{
    m_num_output = type;
}


/*
========================
bindState

 bind current state to GL.
 returns true, if successfull
========================
*/
bool CShader::bindState( VertexAttribLocations & attribs )
{
	if( m_program != 0 && m_linked )
	{
		glUseProgram( m_program );
		attribs = m_attribLocations;

		for( int i = 0 ; i < m_activeUniforms.size() ; i++ )
		{
			updateTimeVariable( m_activeUniforms[i] );
			m_activeUniforms[i].applyToGL();
		}

		return true;
	}

	attribs = VertexAttribLocations();
	glUseProgram( 0 );
	return false;
}


/*
========================
updateTimeVariable

 checks wether this is the 'time' variable and updates it.
========================
*/
void CShader::updateTimeVariable( CUniform & u )
{
	// is this the time variable?
	if( u.getType() == GL_FLOAT &&
		0 == QString("time").compare( u.getName(), Qt::CaseInsensitive ) )
	{
		// read timer and convert to seconds
		u.setValueAsFloat( 0, double(m_timer.elapsed()) * 0.001 );
	}
}


/*
========================
getBuildLog
========================
*/
QString CShader::getBuildLog( void )
{
	return m_log;
}


/*
========================
setupInitialUniforms

 This is called after the program was successfully linked and validated.
========================
*/
void CShader::setupInitialUniforms( void )
{
	GLint n = 0;
	glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &n );

	QVector< CUniform > uniforms;

	// read all active uniforms
	for( int i = 0 ; i < n ; i++ )
	{
		// read out info
		char name[ 256 ] = "\0";
		GLsizei length = 0;
		GLint size = 0;
		GLenum type = 0;
		glGetActiveUniform( m_program, i, sizeof(name), &length, &size, &type, name );
		GLint locus = glGetUniformLocation( m_program, name );

		// add to list
		uniforms.append( CUniform( QString( name ), type, locus ) );
	}

	// replace buffers
	m_oldUniforms = m_activeUniforms;
	m_activeUniforms = uniforms;
}


/*
========================
setupRememberedUniformState

 Scans m_oldUniforms for name and type matches in m_activeUniforms
 and copies thoses values. This makes it unnecessary for the user
 to setup each uniform every time the shader is compiled. This way
 no data is stored in the presentation code.
========================
*/
void CShader::setupRememberedUniformState( void )
{
	for( int i = 0 ; i < m_oldUniforms.size() ; i++ )
	{
		for( int j = 0 ; j < m_activeUniforms.size() ; j++ )
		{
			const CUniform & old = m_oldUniforms[ i ];
			const CUniform & neu = m_activeUniforms[ j ];

			// match name
			if( old.getName().compare( neu.getName(), Qt::CaseSensitive ) )
				continue;

			// match type
			if( old.getType() != neu.getType() )
				continue;

			// do not overwrite the new location!!
			m_activeUniforms[ j ] = CUniform( old, neu.getLocation() );
		}
	}
}


/*
========================
setUniform
========================
*/
void CShader::setUniform( int index, const CUniform & u )
{
	// must be valid
	if( !m_linked || index < 0 || index >= m_activeUniforms.size() )
		return;

	// check for type/name mismatch!
	const CUniform & u2 = m_activeUniforms[ index ];
	if( u2.getName() != u.getName() ||
		u2.getType() != u.getType() ||
		u2.getLocation() != u.getLocation() ) // does not move around by specification...
	{
		return;
	}

	// saves new data
	m_activeUniforms[ index ] = u;
}


/*
========================
getActiveUniforms
========================
*/
int CShader::getActiveUniforms( void )
{
	if( !m_linked )
		return 0;

	return m_activeUniforms.size();
}


/*
========================
getUniform
========================
*/
CUniform CShader::getUniform( int index )
{
	if( m_linked && // must be initialized
		index >= 0 && index < m_activeUniforms.size() )
	{
		return m_activeUniforms[ index ];
	}

	// invalid...
	return CUniform();
}


/*
========================
toGlShaderType

 maps TYPE_xy to GL_xy
========================
*/
int CShader::toGlShaderType( int symbol )
{
	switch( symbol )
	{
	case TYPE_VERTEX:	return GL_VERTEX_SHADER;		break;
	case TYPE_GEOMETRY:	return GL_GEOMETRY_SHADER_EXT;	break;
	case TYPE_FRAGMENT:	return GL_FRAGMENT_SHADER;		break;
	}

	// never get here...
	assert( 0 );
	return 0;
}


/*
========================
isShaderTypeAvailable
========================
*/
bool CShader::isShaderTypeAvailable( int type )
{
	switch( type )
	{
	case TYPE_VERTEX:
	case TYPE_FRAGMENT:
		return true;

	case TYPE_GEOMETRY:
		return m_geometryShaderAvailable;
	}

	return false;
}


/*
========================
getShaderTypeName
========================
*/
QString IShader::getShaderTypeName( int symbol )
{
	switch( symbol )
	{
	case TYPE_VERTEX:	return QString( "Vertex Shader" );		break;
	case TYPE_GEOMETRY:	return QString( "Geometry Shader" );	break;
	case TYPE_FRAGMENT:	return QString( "Fragment Shader" );	break;
	}

	return QString( "<bad shader type>" );
}


/*
========================
logActiveUniforms

 assumes the program is linked.
========================
*/
void CShader::logActiveUniforms( void )
{
	// get count
	GLint n = 0;
	glGetProgramiv( m_program, GL_ACTIVE_UNIFORMS, &n );

	// write headline
	m_log += QString( "\n-----\n\n" );
	m_log += QString( "Active Uniforms:  <index:  name @ location,  type>\n\n" );

	// write infos
	for( int i = 0 ; i < n ; i++ )
	{
		// read infos
		char name[ 256 ] = "\0";
		GLsizei length = 0;
		GLint size = 0;
		GLenum type;
		glGetActiveUniform( m_program, i, sizeof(name), &length, &size, &type, name );
		GLint loc = glGetUniformLocation( m_program, name );

		m_log += QString( "%1:  %2 @ %3,  %4%5\n" ).
			arg( i ).
			arg( name ).
			arg( loc ).
			arg( CUniform::getTypeNameString( type ) ).
			arg( size > 1 ? QString( " [ %1 ]" ).arg( size ) : QString("") );
	}

	m_log += QString( "\n" );
}


/*
========================
logActiveAttributes

 assumes the program is linked
========================
*/
void CShader::logActiveAttributes( void )
{
	GLint n = 0;
	glGetProgramiv( m_program, GL_ACTIVE_ATTRIBUTES, &n );

	// headline
	m_log += QString( "\n-----\n\n" );
	m_log += QString( "Active Attributes:  <index: name @ location,  type>\n\n" );

	// attributes
	for( int i = 0 ; i < n ; i++ )
	{
		// read infos
		char name[ 256 ] = "\0";
		GLsizei length = 0;
		GLint size = 0;
		GLenum type;
		glGetActiveAttrib( m_program, i, sizeof(name), &length, &size, &type, name );
		int locus = glGetAttribLocation( m_program, name );

		// map symbol to string
		QString str = QString( "<bad type %1>" ).arg( type );
		switch( type )
		{
		case GL_FLOAT:		str = QString( "GL_FLOAT" ); break;
		case GL_FLOAT_VEC2: str = QString( "GL_FLOAT_VEC2" ); break;
		case GL_FLOAT_VEC3: str = QString( "GL_FLOAT_VEC3" ); break;
		case GL_FLOAT_VEC4: str = QString( "GL_FLOAT_VEC4" ); break;
		case GL_FLOAT_MAT2: str = QString( "GL_FLOAT_MAT2" ); break;
		case GL_FLOAT_MAT3: str = QString( "GL_FLOAT_MAT3" ); break;
		case GL_FLOAT_MAT4: str = QString( "GL_FLOAT_MAT4" ); break;
		}

		// write log line
		m_log += QString( "%1: %2 @ %3,  %4\n" ).arg( i ).arg( name ).arg( locus ).arg( str );
	}

	m_log += QString( "\n" );
}



