//=============================================================================
/** @file		scene.cpp
 *
 * Implements the IScene interface.
 *
	@internal
	created:	2007-10-24
	last mod:	2008-02-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QtCore/QTime>

#include "application.h"
#include "scene.h"
#include "model.h"
#include "light.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"


//=============================================================================
//	ITextureState implementation
//=============================================================================

/** Implementation of the ITextureState interface.
 */
class CTextureState : public ITextureState
{
public:
	/** Constructs a CTextureState object.
	 * The object must be initialized before use.
	 */
	CTextureState( void );
	virtual ~CTextureState( void ); ///< Destructor.

	// ITextureState interface
	int  getMaxTextureUnits( void ) { return m_numTextureUnits; }
	void setTexture( int textureUnit, const QImage & image );
	void setBilinearFilter( bool enable ) { m_bilinearFilter = enable; }

	/** Initializes the object.
	 * After this call, the object can be used to manage textures.
	 */
	void init( void );

	/** Frees resources and cleans up state.
	 * Must be called before destruction.
	 */
	void shutdown( void );

	/** Binds the textures to the OpenGL context.
	 */
	void bindState( void );

private:

	// uploads the image to the active texture unit.
	void uploadTexture( const QImage & image );

	// use bilinear filtering?
	bool m_bilinearFilter;

	GLuint*	m_textures; // [ m_numTextureUnits ]
	GLint   m_numTextureUnits;
};


// construction
CTextureState::CTextureState( void )
{
	m_textures = NULL;
	m_numTextureUnits = 0;
	m_bilinearFilter = false;
}

CTextureState::~CTextureState( void )
{
}


/*
========================
init
========================
*/
void CTextureState::init( void )
{
	// if not yet initialized
	if( m_numTextureUnits == 0 )
	{
		glGetIntegerv( GL_MAX_TEXTURE_UNITS, &m_numTextureUnits );

		m_textures = new GLuint [ m_numTextureUnits ];
		memset( m_textures, 0, sizeof(GLuint) * m_numTextureUnits );
	}
}


/*
========================
shutdown
========================
*/
void CTextureState::shutdown( void )
{
	for( int i = 0 ; i < m_numTextureUnits ; i++ )
	{
		if( m_textures[ i ] != 0 )
			glDeleteTextures( 1, &m_textures[ i ] );
	}

	SAFE_DELETE_ARRAY( m_textures );
	m_numTextureUnits = 0;
}


/*
========================
bindState
========================
*/
void CTextureState::bindState( void )
{
	for( int i = 0 ; i < m_numTextureUnits ; i++ )
	{
		if( m_textures[ i ] != 0 )
		{
			glActiveTexture( GL_TEXTURE0 + i );
			glBindTexture( GL_TEXTURE_2D, m_textures[ i ] );
			glDisable( GL_TEXTURE_2D ); // can only be accessed by shaders.

			// set filter mode
			GLint filter = m_bilinearFilter ? GL_LINEAR : GL_NEAREST;
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
		}
	}

	// restore state
	glActiveTexture( GL_TEXTURE0 );
}


/*
========================
setTexture
========================
*/
void CTextureState::setTexture( int textureUnit, const QImage & image )
{
	// validate index
	if( textureUnit < 0 || textureUnit >= m_numTextureUnits )
		return;

	//
	// invalid, free texture object!
	//
	if( image.isNull() )
	{
		// unbind current texture object
		glActiveTexture( GL_TEXTURE0 + textureUnit );
		glBindTexture( GL_TEXTURE_2D, 0 );

		// destroy texture object
		if( m_textures[ textureUnit ] != 0 )
		{
			glDeleteTextures( 1, &m_textures[ textureUnit ] );
			m_textures[ textureUnit ] = 0;
		}
	}

	//
	// valid, upload image
	//
	else
	{
		// create texture object if necessary
		if( m_textures[ textureUnit ] == 0 )
		{
			glGenTextures( 1, &m_textures[ textureUnit ] );
		}

		// setup GL state
		glActiveTexture( GL_TEXTURE0 + textureUnit );
		glBindTexture  ( GL_TEXTURE_2D, m_textures[ textureUnit ] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		// now upload the image
		uploadTexture( image );
	}

	// restore state
	glActiveTexture( GL_TEXTURE0 );
}


/*
========================
uploadTexture

 uploads the image to the active texture unit.
========================
*/
void CTextureState::uploadTexture( const QImage & image )
{
	// rescale to power of 2
	// -> this is necessary due to hardware limits!!
	// -> some drivers report GL2.0 but do not support NPOT textures.
	GLint w,h, maxSize = 64;
	glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxSize );
	const char* extensions = (const char*)glGetString( GL_EXTENSIONS );
	if( NULL == strstr( extensions, "GL_ARB_texture_non_power_of_two" ) )
	{
		for( w = 1 ; w < image.width()  ; w *= 2 ) ;
		for( h = 1 ; h < image.height() ; h *= 2 ) ;
	}
	else
	{
		w = image.width();
		h = image.height();
	}
	w = qMin( w, maxSize );
	h = qMin( h, maxSize );

	// convert image into a useful format
	QImage fixedImage = image.
		convertToFormat( QImage::Format_ARGB32 ).
		rgbSwapped().
		scaled( w,h );

	// upload image data
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 
		fixedImage.width(), fixedImage.height(), 0, 
		GL_RGBA, GL_UNSIGNED_BYTE, fixedImage.bits() );
}



//=============================================================================
//	ICameraState implementation
//=============================================================================

/** Implementation of the ICameraState interface.
 */
class CCameraState : public ICameraState
{
public:
	/** Constructs a CCameraState object.
	 * The constructed object is ready to use.
	  */
	CCameraState( void );

	// ICameraState interface
	void resetCamera( void );
	void setFovY( float fovY ) { m_fovY = fovY; }
	void setProjectionMode( projMode_e mode );
	void setCameraRotation( const mat4_t & matrix ) { m_rotation = matrix; }
	void getCameraRotation( mat4_t & matrix ) { matrix = m_rotation; }
	void setCameraTranslation( const vec3_t & t ) { m_translation = t; }
	void getCameraTranslation( vec3_t & t ) { t = m_translation; }

	/** Multiplies the camera's projection matrix to the current GL_PROJECTION matrix.
	 * The resulting matrix is P1 = P0 * M.
	 * @param worldRadius Bounding radius of the wolrd's geometry.
	 *					  Must be greater than zero.
	 */
	void applyProjectionMatrix( float worldRadius );

	/** Multiplies the camera's transformation to the current GL_MODELVIEW matrix.
	 * The resulting matrix is P1 = P0 * M.
	 */
	void applyModelViewMatrix( void );

	/** Returns the camera transformation as a model view matrix.
	 * @param matrix Pointer to a 16 element float array representing a 4x4 column major order matrix. 
	 *				 The requested matrix will be strored in this buffer.
	 */
	void getModelViewMatrix( mat4_t & matrix );

private:

	// projection matrix helpers
	void setupFrustum( void );
	void setupOrtho( float worldRadius );

	float		m_fovY;
	projMode_e	m_projectionMode;
	mat4_t		m_rotation;
	vec3_t		m_translation;
};


// construction
CCameraState::CCameraState( void )
{
	m_projectionMode = PROJECT_FRUSTUM;
	m_fovY = 50.0f;

	resetCamera();
}


/*
========================
applyProjectionMatrix
========================
*/
void CCameraState::applyProjectionMatrix( float worldRadius )
{
	// call the matching matrix creation helper.
	if( m_projectionMode == PROJECT_ORTHO ) {
		setupOrtho( worldRadius );
	} else {
		setupFrustum();
	}
}


/*
========================
applyModelViewMatrix
========================
*/
void CCameraState::applyModelViewMatrix( void )
{
	mat4_t m;
	getModelViewMatrix( m ); // avoids one matrix multiplication...
	glMultMatrixf( m.toConstFloatPointer() );
}


/*
========================
getModelViewMatrix
========================
*/
void CCameraState::getModelViewMatrix( mat4_t & matrix )
{
	matrix = m_rotation; // first rotate...
	matrix.setTranslationVector( m_translation ); /// ... then translate
}


/*
========================
setProjectionMode
========================
*/
void CCameraState::setProjectionMode( projMode_e mode )
{
	switch( mode )
	{
	case PROJECT_ORTHO:
	case PROJECT_FRUSTUM:
		m_projectionMode = mode;
		break;
	}
}


/*
========================
resetCamera
========================
*/
void CCameraState::resetCamera( void )
{
	// reset to identity.
	m_rotation = Matrix4x4();

	m_translation = vec3_t( 0,0, -2.5f );
}


/*
========================
setupFrustumView
========================
*/
void CCameraState::setupFrustum( void )
{
	// tuning constants
	static const double pi = 4.0 * atan( 1.0 );
	double zNear = 0.01f;
	double zFar = 20.0f;

	double a,b;

	// do range check
	if( m_fovY <= 0.0f && m_fovY >= 180.0f )
	{
		m_fovY = 50.0f;
	}

	// get current viewport
	GLfloat vp[4];
	glGetFloatv( GL_VIEWPORT, vp );
	double width  = vp[ 2 ];
	double height = vp[ 3 ];

	a = tan( m_fovY * pi / 360.0 );
	b = a * width / height;

	double xmax = zNear * b;
	double xmin = -xmax;
	double ymax = zNear * a;
	double ymin = -ymax;

	// setup frustum
	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}


/*
========================
setupOrthoView

 worldRadius is the radius of the bounding sphere of the complete scene.
========================
*/
void CCameraState::setupOrtho( float worldRadius )
{
	if( worldRadius <= 0.0f )
		worldRadius = 1.0f;

	float halfWorldSizeX = 0.0f;
	float halfWorldSizeY = 0.0f;

	// get viewport dimensions
	GLfloat vp[ 4 ];
	glGetFloatv( GL_VIEWPORT, vp );
	float width  = vp[ 2 ];
	float height = vp[ 3 ];

	// add some bonus space
	float radius = worldRadius * 1.2f;
	float aspect = width / height;

	// calc projection params
	if( width > height ) {
		halfWorldSizeX = radius * aspect;
		halfWorldSizeY = radius;
	} else {
		halfWorldSizeX = radius;
		halfWorldSizeY = radius / aspect;
	}

	// setup projection matrix
	glOrtho( -halfWorldSizeX, halfWorldSizeX,
			 -halfWorldSizeY, halfWorldSizeY, -9999, 9999 );
}



//=============================================================================
//	ILightingState implementation
//=============================================================================

/** Implements the ILightingState interface.
 */
class CLightingState : public ILightingState
{
public:
	/** Constructs a CLightingState object.
	 * The constructed object must be initialized before use.
	 */
	CLightingState( void );

	/** Initializes the object.
	 * Loads required resources and makes the object ready for use.
	 * This call also loads the default lighting state for this object.
	 */
	void init( void );

	/** Frees resources and prepares the object for destruction.
	 * Must be called before deleting a n initialized CLightingState object.
	 */
	void shutdown( void );


	/** Applies the lighting state to the OpenGL context.
	 * This call applies the lighting-enables bit and the light sources.
	 * @warning This call needs one free element of the OpenGL modelview matrix stack.
	 * @param viewMatrix Transformation matrix from world space to eye space.
	 * @param autoRotateMatrix This matrix is applied to the light position if the light's auto-rotate flag is set.
	 */
	void bindLights( const mat4_t & viewMatrix, const mat4_t & autoRotateMatrix );

	/** Applies the material parameters to the OpenGL context.
	 */
	void bindMaterial( void );


	/** Draws a sphere for every enabled light source.
	 * The spheres are specified in world coordinates.
	 * @warning This call needs one free elements of the OpenGL modelview matrix stack.
	 * @param viewMatrix Transformation matrix form world space to eye space.
	 * @param autoRotateMatrix This matrix is applied to the light position if the light's auto-rotate flag is set.
	 */
	void drawLights( const mat4_t & viewMatrix, const mat4_t & autoRotateMatrix );


	// ILightingState interface
	void setShowLights( bool enable ) { m_showLights = enable; }
	void setLight( int index, const CLight & l );
	void setMaterial( const CMaterial & m ) { m_material = m; }
	void setLightingEnabled( bool enable );
	bool getLight( int index, CLight & l ) const;
	bool getLightingEnabled( void ) const;
	void getMaterial( CMaterial & m ) const { m = m_material; }

private:

	// helpers
	void initDefaultLightState( void );
	void drawLightSource( const CLight & l );

	bool		m_enableLighting;
	bool		m_showLights;
	CLight		m_lights[ MAX_LIGHTS ];
	CMaterial	m_material;
	IModel*		m_showLightsModel;
};


// construction
CLightingState::CLightingState( void )
{
	m_showLights = false;
	m_showLightsModel = NULL;

	m_enableLighting = false;

	// load defualt state for all light sources
	initDefaultLightState();
}


/*
========================
init
========================
*/
void CLightingState::init( void )
{
	// create light visualization model
	if( m_showLightsModel == NULL ) {
		m_showLightsModel = IModel::createSphere( 4, 8, 0.1f );
	}

	initDefaultLightState();
}


/*
========================
shutdown
========================
*/
void CLightingState::shutdown( void )
{
	SAFE_DELETE( m_showLightsModel );
}


/*
========================
bindLights
========================
*/
void CLightingState::bindLights( const mat4_t & viewMatrix, const mat4_t & autoRotateMatrix )
{
	// needs the modelview matrix!
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity(); // positions are transformed manually.

	for( int i = 0 ; i < ILightingState::MAX_LIGHTS ; i++ )
	{
		const CLight & l = m_lights[ i ];
		vec4_t position = l.getPosition();

		// auto-rotate light
		if( l.getAutoRotate() ) {
			position = autoRotateMatrix * position;
		}

		// transform the light position from wolrd space to eye space.
		if( !l.getLockedToCamera() ) {
			position = viewMatrix * position;
		}


		glLightfv( GL_LIGHT0 + i, GL_POSITION, position.toFloatPointer() );
		glLightfv( GL_LIGHT0 + i, GL_AMBIENT,  l.getAmbient().toFloatPointer()  );
		glLightfv( GL_LIGHT0 + i, GL_DIFFUSE,  l.getDiffuse().toFloatPointer()  );
		glLightfv( GL_LIGHT0 + i, GL_SPECULAR, l.getSpecular().toFloatPointer() );

		if( l.getEnabled() ) {
			glEnable( GL_LIGHT0 + i );
		} else {
			glDisable( GL_LIGHT0 + i );
		}
	}

	glPopMatrix();

	//
	// lighting enabled bit
	//
	if( m_enableLighting ) {
		glEnable( GL_LIGHTING );
	} else {
		glDisable( GL_LIGHTING );
	}
}


/*
========================
bindMaterial
========================
*/
void CLightingState::bindMaterial( void )
{
	glMaterialfv( GL_FRONT_AND_BACK, GL_EMISSION,  m_material.getEmission().toFloatPointer() );
	glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT,   m_material.getAmbient ().toFloatPointer() );
	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE,   m_material.getDiffuse ().toFloatPointer() );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR,  m_material.getSpecular().toFloatPointer() );
	glMaterialf ( GL_FRONT_AND_BACK, GL_SHININESS, m_material.getSpecularExponent() );

	// color material...
	if( m_material.getUseVertexColor() )
	{
		glEnable( GL_COLOR_MATERIAL );
		glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	}
	else
	{
		glDisable( GL_COLOR_MATERIAL );
	}
}


/*
========================
drawLights
========================
*/
void CLightingState::drawLights( const mat4_t & viewMatrix, const mat4_t & autoRotateMatrix )
{
	int i;

	// not enabled or no initialized.
	if( !m_showLights || m_showLightsModel == NULL )
		return;

	glMatrixMode( GL_MODELVIEW );

	//
	// draw lights specified in eye space
	//
	for( i = 0 ; i < MAX_LIGHTS ; i++ )
	{
		CLight & l = m_lights[ i ];

		// only draw enabled lights
		if( !l.getEnabled() )
			continue;

		glPushMatrix();
		glLoadIdentity();

		// transform form world to eye space
		if( !l.getLockedToCamera() ) {
			glMultMatrixf( viewMatrix.toConstFloatPointer() );
		}

		// apply auto-rotation
		if( l.getAutoRotate() ) {
			glMultMatrixf( autoRotateMatrix.toConstFloatPointer() );
		}

		drawLightSource( l );

		glPopMatrix();
	}
}


/*
========================
drawLightSource

 sends the sphere model for a single light source to OpenGL.
========================
*/
void CLightingState::drawLightSource( const CLight & l )
{
	if( m_showLightsModel != NULL )
	{
		vec4_t color = l.getDiffuse();
		vec4_t position = l.getPosition();
		glTranslatef( position.x, position.y, position.z );
		m_showLightsModel->render( NULL, &color );
	}
}


/*
========================
getLightingEnabled
========================
*/
bool CLightingState::getLightingEnabled( void ) const
{
	return m_enableLighting;
}


/*
========================
getLight
========================
*/
bool CLightingState::getLight( int index, CLight & l ) const
{
	if( index < 0 || index >= MAX_LIGHTS )
		return false;

	l = m_lights[ index ];
	return true;
}


/*
========================
setLightingEnabled
========================
*/
void CLightingState::setLightingEnabled( bool enable )
{
	m_enableLighting = enable;
}


/*
========================
setLight
========================
*/
void CLightingState::setLight( int index, const CLight & l )
{
	if( index >= 0 && index < MAX_LIGHTS )
	{
		m_lights[ index ] = l;
	}
}


/*
========================
initDefaultLightState

 this should match the GL specification.
 -> makes all lights point lights.
========================
*/
void CLightingState::initDefaultLightState( void )
{
	for( int i = 0 ; i < MAX_LIGHTS ; i++ )
	{
		CLight & l = m_lights[ i ];

		l.setPosition( vec4_t( 0.0f, 0.0f, 2.0f, 1.0f ) ); // point light source
		l.setAmbient ( vec4_t( 0.0f, 0.0f, 0.0f, 1.0f ) );

		if( i == 0 )
		{
			l.setEnabled ( true );
			l.setDiffuse ( vec4_t( 1.0f, 1.0f, 1.0f, 1.0f ) );
			l.setSpecular( vec4_t( 1.0f, 1.0f, 1.0f, 1.0f ) );
		}

		else
		{
			l.setEnabled ( false );
			l.setDiffuse ( vec4_t( 0.0f, 0.0f, 0.0f, 1.0f ) );
			l.setSpecular( vec4_t( 0.0f, 0.0f, 0.0f, 1.0f ) );
		}
	}

	// initial material state
	m_material.setEmission( vec4_t( 0.0f, 0.0f, 0.0f, 1.0f ) );
	m_material.setAmbient ( vec4_t( 0.2f, 0.2f, 0.2f, 1.0f ) );
	m_material.setDiffuse ( vec4_t( 0.5f, 0.5f, 0.0f, 1.0f ) );
	m_material.setSpecular( vec4_t( 0.0f, 0.0f, 0.5f, 1.0f ) );
	m_material.setSpecularExponent( 20.0f );
	m_material.setUseVertexColor( false );
}


//=============================================================================
//	IScene implementation
//=============================================================================

/** Implementation of IScene.
 * This class doensn't add new functionality. It is only used to hide
 * the class definition from the interface definition.
 */
class CScene : public IScene
{
public:
	CScene( void );
	virtual ~CScene( void );

	// initialiazation
	void init( void );
	void shutdown( void );

	// rendering
	void render( void );

	// accessing sub objects
	ICameraState*   getCameraState( void ) { return &m_camera; }
	ILightingState* getLightingState( void ) { return &m_lighting; }
	ITextureState*  getTextureState( void ) { return &m_textures; }
	IShader*		getShader( void ) { return m_shader; }

	// current model
	void    setCurrentModel( IModel* model ) { m_model = model; }
	IModel* getCurrentModel( void ) { return m_model; }

	// viewport
	void setClearColor( const vec4_t & color ) { m_clearColor = color; }

	// state flags
	void setUseProgram( bool enable ) { m_useProgram = enable; }
	void setWireframe( bool enable ) { m_enableWireframe = enable; }
	void setBackFaceCulling( bool enable ) { m_enableBFC = enable; }
	void setShowOrigin ( bool enable ) { m_showOrigin = enable; }
	void setShowNormals( bool enable ) { m_showNormals = enable; }
	void setShowBoundingBox( bool enable ) { m_showBoundingBox = enable; }
	void setShowTangents( bool enable ) { m_showTangents = enable; }


private:

	// misc helpers
	void drawTestModel( const mat4_t & lightRotateMatrix );
	void drawHelperGeometry( const mat4_t & lightRotateMatrix );
	void drawOrigin( void );
	void drawBoundingBox( const vec3_t & mins, const vec3_t & maxs );
	void calcLightAutoRotateMatrix( mat4_t & m );

	// state flags
	bool m_enableBFC; // back face culling
	bool m_enableWireframe;
	bool m_useProgram;
	bool m_showOrigin;
	bool m_showNormals;
	bool m_showBoundingBox;
	bool m_showTangents;

	// sub-objects
	CTextureState  m_textures;
	CCameraState   m_camera;
	CLightingState m_lighting;

	// time since initialization
	QTime m_time;

	// viewport clear color
	vec4_t m_clearColor;

	// current test model
	IModel*	m_model;

	// test model shader
	IShader* m_shader;
};


// construction
CScene::CScene( void )
{
	// create sub-objects.
	m_shader = IShader::create();

	m_enableBFC = false;
	m_enableWireframe = false;
	m_useProgram = true;
	m_showOrigin = false;
	m_showNormals = false;
	m_showBoundingBox = false;
	m_showTangents = false;

	m_model = NULL;
}

CScene::~CScene( void )
{
	SAFE_DELETE( m_shader );
}


/*
========================
create
========================
*/
IScene* IScene::create( void )
{
	return new CScene();
}


/*
========================
init
========================
*/
void CScene::init( void )
{
	// initialize sub-objects
	m_shader->init();
	m_textures.init();
	m_lighting.init();

	// start the timer
	m_time.start();
}


/*
========================
shutdown
========================
*/
void CScene::shutdown( void )
{
	// shutdown sub-objects.
	m_lighting.shutdown();
	m_textures.shutdown();
	m_shader->shutdown();
}


/*
========================
render

 binds current state and sends current geometry to GL.
========================
*/
void CScene::render( void )
{
	// clear the viewport
	glClearColor( m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w );
	glClearDepth( 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glShadeModel( GL_SMOOTH );
	glDepthFunc( GL_LEQUAL );

	// light rotation matrix
	mat4_t lightRotate;
	calcLightAutoRotateMatrix( lightRotate );

	//
	// setup projection
	//
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	m_camera.applyProjectionMatrix( m_model != NULL ? m_model->getBoundingRadius() : 1.0f );

	// setup camera
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	m_camera.applyModelViewMatrix();

	// draw something
	drawTestModel( lightRotate );
	drawHelperGeometry( lightRotate );
}


/*
========================
drawTestModel

 setups state and draws the test model.
========================
*/
void CScene::drawTestModel( const mat4_t & lightRotateMatrix )
{
	// no model...
	if( m_model == NULL )
		return;

	VertexAttribLocations attribs;
	bool programAvailable = false;
	mat4_t viewMatrix;

	// setup lighting
	m_camera.getModelViewMatrix( viewMatrix );
	m_lighting.bindLights( viewMatrix, lightRotateMatrix );
	m_lighting.bindMaterial();

	// setup textures
	m_textures.bindState();

	//
	// setup misc state
	//
	glEnable( GL_DEPTH_TEST );
	glPolygonMode( GL_FRONT_AND_BACK, m_enableWireframe ? GL_LINE : GL_FILL );
	if( m_enableBFC ) {
		glEnable( GL_CULL_FACE );
	} else {
		glDisable( GL_CULL_FACE );
	}

	//
	// try using the shader program for rendering.
	//
	if( m_useProgram ) {
		programAvailable = m_shader->bindState( attribs );
	}
	if( !programAvailable ) { // not available, use fixed function pipeline.
		glUseProgram( 0 );
	}

	// now render the model
	m_model->render( &attribs );
}


/*
========================
drawHelperGeometry

 draws origin, lights, normals, etc.
========================
*/
void CScene::drawHelperGeometry( const mat4_t & lightRotateMatrix )
{
	mat4_t m;
	vec3_t mins,maxs;

	// go to support rendering state
	glUseProgram( 0 );
	glDisable( GL_LIGHTING );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	//
	// model related stuff
	//
	if( m_model != NULL )
	{
		// draw normals if requested
		if( m_showNormals ) {
			m_model->renderNormals();
		}

		// show tangent space basis
		if( m_showTangents ) {
			m_model->renderTangents();
		}

		// draw bounding box
		if( m_showBoundingBox ) {
			m_model->getBoundingBox( mins, maxs );
			drawBoundingBox( mins, maxs );
		}
	}

	// draw origin - orientation helper!
	if( m_showOrigin ) {
		drawOrigin();
	}

	//
	// light sources
	//
	m_camera.getModelViewMatrix( m );
	m_lighting.drawLights( m, lightRotateMatrix );
}


/*
========================
calcLightAutoRotateMatrix
========================
*/
void CScene::calcLightAutoRotateMatrix( mat4_t & m )
{
	// abuse the matrix stack...
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	float t = float(m_time.elapsed()) / 1000.0f;
	float angle0 = t * 70;
	float angle1 = t * 60;

	glRotatef( angle0, 0, 0, 1 );
	glRotatef( angle1, 0, 1, 0 );
	glGetFloatv( GL_MODELVIEW_MATRIX, m.toFloatPointer() );

	glPopMatrix();
}


/*
========================
drawBoundingBox
========================
*/
void CScene::drawBoundingBox( const vec3_t & mins, const vec3_t & maxs )
{
	glBegin( GL_LINES );

	// X axis
	glColor3f( 1,0,0 );
	glVertex3f( mins.x, mins.y, mins.z ); glVertex3f( maxs.x, mins.y, mins.z );
	glVertex3f( mins.x, maxs.y, mins.z ); glVertex3f( maxs.x, maxs.y, mins.z );
	glVertex3f( mins.x, mins.y, maxs.z ); glVertex3f( maxs.x, mins.y, maxs.z );
	glVertex3f( mins.x, maxs.y, maxs.z ); glVertex3f( maxs.x, maxs.y, maxs.z );

	// Y axis
	glColor3f( 0,1,0 );
	glVertex3f( mins.x, mins.y, mins.z ); glVertex3f( mins.x, maxs.y, mins.z );
	glVertex3f( maxs.x, mins.y, mins.z ); glVertex3f( maxs.x, maxs.y, mins.z );
	glVertex3f( mins.x, mins.y, maxs.z ); glVertex3f( mins.x, maxs.y, maxs.z );
	glVertex3f( maxs.x, mins.y, maxs.z ); glVertex3f( maxs.x, maxs.y, maxs.z );

	// Z axis
	glColor3f( 0,0,1 );
	glVertex3f( mins.x, mins.y, mins.z ); glVertex3f( mins.x, mins.y, maxs.z );
	glVertex3f( maxs.x, mins.y, mins.z ); glVertex3f( maxs.x, mins.y, maxs.z );
	glVertex3f( mins.x, maxs.y, mins.z ); glVertex3f( mins.x, maxs.y, maxs.z );
	glVertex3f( maxs.x, maxs.y, mins.z ); glVertex3f( maxs.x, maxs.y, maxs.z );

	glEnd();
}


/*
========================
drawOrigin
========================
*/
void CScene::drawOrigin( void )
{
	glLineWidth( 3 );

	glBegin( GL_LINES );
		glColor3f( 1,0,0 );
		glVertex3f( 0,0,0 ); glVertex3f( 2,0,0 );
		glColor3f( 0,1,0 );
		glVertex3f( 0,0,0 ); glVertex3f( 0,2,0 );
		glColor3f( 0,0,1 );
		glVertex3f( 0,0,0 ); glVertex3f( 0,0,2 );
	glEnd();

	glLineWidth( 1 );
}


