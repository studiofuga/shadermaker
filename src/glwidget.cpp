//=============================================================================
/** @file		glwidget.cpp
 *
 * Implements CGLWidget.
 *
	@internal
	created:	2007-11-30
	last mod:	2008-03-30

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QMessageBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>

#include "application.h"
#include "glwidget.h"
#include "camera.h"


//=============================================================================
//	CGLWidget implementation
//=============================================================================

// construction
CGLWidget::CGLWidget( const QGLFormat & format, ICameraState* cs )
 : QGLWidget( format )
{
	// make sure the GL context is active
	makeCurrent();

	m_initSucceeded = false;
	m_cameraState = cs;

	m_fpsCounter = 0;
	m_fpsValue = 0;
	m_fpsLastPeriod = 0;

	setFocusPolicy( Qt::StrongFocus );
	setMouseTracking( true );
}

CGLWidget::~CGLWidget( void )
{
	makeCurrent();
}


/*
========================
sizeHint
========================
*/
QSize CGLWidget::sizeHint( void ) const
{
	return QSize( 512, 1024 );
}


/*
========================
minimumSizeHint
========================
*/
QSize CGLWidget::minimumSizeHint( void ) const
{
	return QSize( 128, 128 );
}


/*
========================
initializeGL

 - the GL context is current (see Qt spec...)
 - the GL_VERSION string will give us the info we need to know
========================
*/
void CGLWidget::initializeGL( void )
{
	// check for OpenGL 2.0
//	if( !( GLEE_VERSION_2_0 ) ) // doesn't work.
//	if( !( context()->format().openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0 ) ) // doesn't work either.
	if( !validateDriverVersion( 2, 0 ) ) // maybe this will work...
	{
		QMessageBox::warning( NULL, tr( CONFIG_STRING_ERRORDLG_TITLE ),
			tr( "OpenGL 2.0 not available\n\n"
			    "This program requires an OpenGL 2.0 compatible driver installed.\n\n"
			    "The current OpenGL driver's GL_VERSION string is:\n"
			    "%1" ).arg( (const char*)glGetString( GL_VERSION ) ),
			QMessageBox::Ok, QMessageBox::Ok );
		return;
	}

	// start FPS timer
	m_fpsTimer.start();
	m_fpsLastPeriod = m_fpsTimer.elapsed() - 1000;

	// start periodic refesh timer
	startTimer( CONFIG_REFRESH_INTERVAL );
	m_initSucceeded = true;
}


/*
========================
paintGL
========================
*/
void CGLWidget::paintGL( void )
{
	if( !m_initSucceeded )
		return;

	glViewport( 0,0, m_viewportSize.width(), m_viewportSize.height() );

	emit render();

	drawFPS();
}


/*
========================
drawFPS

 This updates the FPS counter and displays current frame rate.
 If VSYNC is enabled, then we get very unprecise results.
 We must include SwapBuffers() in execution time, because most
 rendering work is done there.
========================
*/
void CGLWidget::drawFPS( void )
{
	// update value
	int now = m_fpsTimer.elapsed();
	if( now - m_fpsLastPeriod >= 1000 )
	{
		m_fpsValue = m_fpsCounter;
		m_fpsCounter = 0;
		m_fpsLastPeriod = now;
	}

	// count this frame
	m_fpsCounter++;

	// format text
	QString text = QString( "%1 FPS" ).arg( m_fpsValue );
	int length = fontMetrics().width( text );

	// setup state and render text
	glUseProgram( 0 );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );
	glColor3f( 1,1,1 );
	renderText( m_viewportSize.width() - length - 1, font().pointSize() + 1, text );
}


/*
========================
resizeGL
========================
*/
void CGLWidget::resizeGL( int width, int height )
{
	m_viewportSize = QSize( width, height );
}


/*
========================
keyPressEvent
========================
*/
void CGLWidget::keyPressEvent( QKeyEvent* event )
{
	// use ESC to resete the camera
	if( event->key() == Qt::Key_Escape )
	{
		m_cameraState->resetCamera();
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


/*
========================
mouseMoveEvent
========================
*/
void CGLWidget::mouseMoveEvent( QMouseEvent* event )
{
	int deltaX = event->x() - m_lastMousePosition.x();
	int deltaY = event->y() - m_lastMousePosition.y();

	int buttons = event->buttons();
	int controlFlag = ( event->modifiers() & Qt::MetaModifier ); // MacOS X

	//
	// back to front
	//
	if( controlFlag || ( buttons & Qt::RightButton ) )
	{
		vec3_t t;
		m_cameraState->getCameraTranslation( t );
		t.z += 0.02f * float(deltaY);
		m_cameraState->setCameraTranslation( t );
	}

	//
	// left/right up/down
	//
	else if ( buttons & Qt::MidButton )
	{
		vec3_t t;
		m_cameraState->getCameraTranslation( t );
		t.x += 0.01f * float(deltaX);
		t.y -= 0.01f * float(deltaY);
		m_cameraState->setCameraTranslation( t );
	}

	//
	// rotation
	//
	else if( buttons & Qt::LeftButton )
	{
		mat4_t m;
		m_cameraState->getCameraRotation( m );

		// use the matrix stack for this multiplication...
		glPushMatrix();
		glLoadIdentity();

		// apply new rotation after old rotation
		glRotatef( 0.5f * float(deltaX), 0.0f, 1.0f, 0.0f );
		glRotatef( 0.5f * float(deltaY), 1.0f, 0.0f, 0.0f );
		glMultMatrixf( m.toConstFloatPointer() );

		// state update...
		glGetFloatv( GL_MODELVIEW_MATRIX, m.toFloatPointer() );
		m_cameraState->setCameraRotation( m );
		glPopMatrix();
    }

	// save state and accept event.
	m_lastMousePosition = event->pos();
	event->accept();
}


/*
========================
timerEvent
========================
*/
void CGLWidget::timerEvent( QTimerEvent* )
{
	updateGL();
}


/*
========================
validateDriverVersion

 OpenGL spec:

  GL_VERSION := <version-number><space><implementation-specific>
  <version-number> := <major>.<minor> | <major>.<minor>.<release>
  <major> := integer number
  <minor> := integer number
========================
*/
bool CGLWidget::validateDriverVersion( int majorRequired, int minorRequired )
{
	int i;
	int major = 0; // detected
	int minor = 0;

	// get version string
	const char* versionString = (const char*)glGetString( GL_VERSION );

	// parse major number
	for( i = 0 ; versionString[i] != '\0' && versionString[i] != ' ' ; i++ )
	{
		char c = versionString[i];

		if( c < '0' || c > '9' )
			break;

		major *= 10;
		major += ( c - '0' );
	}

	if( versionString[i] != '.' ) {
		goto parse_error;
	}

	// parse minor number
	for( i++ ; versionString[i] != '\0' && versionString[i] != ' ' ; i++ )
	{
		char c = versionString[i];

		if( c < '0' || c > '9' )
			break;

		minor *= 10;
		minor += ( c - '0' );
	}

	if( versionString[i] != '.' && versionString[i] != ' ' ) {
		goto parse_error;
	}

	// now check version numbers!
	if( ( major > majorRequired ) ||
		( major >= majorRequired && minor >= minorRequired ) )
	{
		return true;
	}

	return false;

parse_error:
	QMessageBox::warning( NULL, tr( CONFIG_STRING_ERRORDLG_TITLE ),
			tr( "Error parsing GL_VERSION.\n"
				"The string contains a syntax error.\n" ),
			QMessageBox::Ok, QMessageBox::Ok );
	return false;
}


/*
========================
getDriverInfoString

 fills the GL info widget with text
========================
*/
QString CGLWidget::getDriverInfoString( void ) const
{
	// the context must be valied!
	if( !context()->isValid() )
		return QString( "<invalid OpenGL context>" );

	// read OpenGL infos
	QString text;
	text += tr( "Vendor:   %1\n" ).arg( (const char*)glGetString( GL_VENDOR ) );
	text += tr( "Renderer: %1\n" ).arg( (const char*)glGetString( GL_RENDERER ) );
	text += tr( "Version:  %1\n" ).arg( (const char*)glGetString( GL_VERSION ) );
	text += tr( "\n" );

	// print framebuffer format
	QGLFormat fmt = context()->format();
	text += tr( "Framebuffer Format:\n" );
	text += tr( " RGBA bits:    (%1,%2,%3,%4)\n" ).
				arg( fmt.redBufferSize() ).
				arg( fmt.greenBufferSize() ).
				arg( fmt.blueBufferSize() ).
				arg( fmt.alphaBufferSize() );
	text += tr( " Depth bits:   %1\n" ).arg( fmt.depthBufferSize() );
	text += tr( " Stencil bits: %1\n" ).arg( fmt.stencilBufferSize() );
	text += tr( "\n" );

	// shading language version
	if( fmt.openGLVersionFlags() & fmt.OpenGL_Version_2_0 )
	{
		QString version( (const char*)glGetString( GL_SHADING_LANGUAGE_VERSION ) );
		text += QString( "Shading Language Version: %1\n" ).arg( version );
	}

	// check for geometry shader
	bool hasGeoShader = false;
#ifdef CONFIG_ENABLE_GEOMETRY_SHADER
	hasGeoShader = ( NULL != strstr( (const char*)glGetString( GL_EXTENSIONS ), "GL_EXT_geometry_shader4" ) );
	if( hasGeoShader )
	{
		text += tr( "GL_EXT_geometry_shader4 available\n" );
	}
	else
	{
		text += tr( "GL_EXT_geometry_shader4 not supported\n" );
	}
#endif
	text += tr( "\n" );

	GLint i;
#define PRINT_CONSTANT(Name) \
	glGetIntegerv( Name, &i ); \
	text += tr( #Name " = %1\n" ).arg( i );

	PRINT_CONSTANT( GL_MAX_TEXTURE_SIZE );
	PRINT_CONSTANT( GL_MAX_TEXTURE_UNITS );
	PRINT_CONSTANT( GL_MAX_VERTEX_ATTRIBS );
	PRINT_CONSTANT( GL_MAX_VERTEX_UNIFORM_COMPONENTS );
	PRINT_CONSTANT( GL_MAX_VARYING_FLOATS ); // alias for GL_MAX_VARYING_COMPONENTS_EXT
	PRINT_CONSTANT( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS );
	PRINT_CONSTANT( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS );
	PRINT_CONSTANT( GL_MAX_TEXTURE_IMAGE_UNITS );
	PRINT_CONSTANT( GL_MAX_TEXTURE_COORDS );
	PRINT_CONSTANT( GL_MAX_FRAGMENT_UNIFORM_COMPONENTS );

	if( hasGeoShader )
	{
		PRINT_CONSTANT( GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_EXT );
		PRINT_CONSTANT( GL_MAX_GEOMETRY_VARYING_COMPONENTS_EXT );
		PRINT_CONSTANT( GL_MAX_VERTEX_VARYING_COMPONENTS_EXT );
		PRINT_CONSTANT( GL_MAX_VARYING_COMPONENTS_EXT );
		PRINT_CONSTANT( GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_EXT) ;
		PRINT_CONSTANT( GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT );
		PRINT_CONSTANT( GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_EXT );
	}

#undef PRINT_CONSTANT

	return text;
}


