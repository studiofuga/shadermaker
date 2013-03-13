//=============================================================================
/** @file		programwindow.cpp
 *
 * Implements CProgramWindow.
 *
	@internal
	created:	2007-10-23
	last mod:	2008-02-21

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>

#include "application.h"

#include "programwindow.h"
#include "glwidget.h"
#include "lightwidget.h"
#include "uniformwidget.h"
#include "scenewidget.h"
#include "texturewidget.h"

#include "scene.h"
#include "shader.h"
#include "editor.h"


//=============================================================================
//	CProgramWindow implementation
//=============================================================================

// construction
CProgramWindow::CProgramWindow( void )
{
	// components
	m_scene = IScene::create();
	m_editor = NULL;

	// create misc widgets
	m_tabs = new QTabWidget();

	// create main widgets
	// -> create order == tab order
	createLogWidget();
	createSceneWidget();
	createLightWidget();
	createUniformWidget();
	createTextureWidget();
	createDriverInfoWidget();

	// setup other widgets
	createGlWidget();
	createLayout();

	setWindowTitle( QString( CONFIG_STRING_APPLICATION_TITLE ) );

	m_tabs->setCurrentWidget( m_sceneWidget );

	// setup connections
	connect( m_glWidget, SIGNAL(render()), this, SLOT(render()) );
}

CProgramWindow::~CProgramWindow( void )
{
	SAFE_DELETE( m_scene );
	SAFE_DELETE( m_editor ); // this is not a child window!
}


/*
========================
createGlWidget
========================
*/
void CProgramWindow::createGlWidget( void )
{
	// make sure we get depth and alpha.
	QGLFormat fmt;
	fmt.setDepth( true );
	fmt.setAlpha( true );
	fmt.setRedBufferSize( 8 );
	fmt.setGreenBufferSize( 8 );
	fmt.setBlueBufferSize( 8 );
	fmt.setAlphaBufferSize( 8 );
	fmt.setDepthBufferSize( 24 );
	fmt.setSwapInterval( 0 ); // turn off VSync

	m_glWidget = new CGLWidget( fmt, m_scene->getCameraState() );
}


/*
========================
createLightWidget
========================
*/
void CProgramWindow::createLightWidget( void )
{
	m_lighting = new CLightWidget( m_scene->getLightingState() );
	m_tabs->addTab( m_lighting, tr( "Lighting" ) );

	// split into two tabs, takes less space...
	m_material = new CMaterialWidget( m_scene->getLightingState() );
	m_tabs->addTab( m_material, tr( "Material" ) );
}


/*
========================
createSceneWidget
========================
*/
void CProgramWindow::createSceneWidget( void )
{
	m_sceneWidget = new CSceneWidget( m_scene );
	m_tabs->addTab( m_sceneWidget, tr( "Scene" ) );
}


/*
========================
createUniformWidget
========================
*/
void CProgramWindow::createUniformWidget( void )
{
	m_uniform = new CUniformWidget( m_scene->getShader() );
	m_tabs->addTab( m_uniform, tr( "Uniforms" ) );
}


/*
========================
createTextureWidget
========================
*/
void CProgramWindow::createTextureWidget( void )
{
	m_texture = new CTextureWidget( m_scene->getTextureState(), m_scene->getShader() );
	m_tabs->addTab( m_texture, tr( "Textures" ) );
}


/*
========================
createLogWidget
========================
*/
void CProgramWindow::createLogWidget( void )
{
	QFont font;
	font.setFamily( tr( CONFIG_EDITOR_FONT_NAME ) );
	font.setFixedPitch( true );
	font.setPointSize( 10 );

	m_logging = new QTextEdit();
	m_logging->setLineWrapMode( QTextEdit::NoWrap );
	m_logging->setReadOnly( true );
	m_logging->setFont( font );
	m_logging->setAcceptRichText( false );

	m_tabs->addTab( m_logging, tr( "Log" ) );
}


/*
========================
createDrivreInfoWidget
========================
*/
void CProgramWindow::createDriverInfoWidget( void )
{
	QFont font;
	font.setFamily( tr( CONFIG_EDITOR_FONT_NAME ) );
	font.setFixedPitch( true );
	font.setPointSize( 10 );

	m_driverInfoWidget = new QTextEdit();
	m_driverInfoWidget->setLineWrapMode( QTextEdit::NoWrap );
	m_driverInfoWidget->setReadOnly( true );
	m_driverInfoWidget->setFont( font );
	m_driverInfoWidget->setAcceptRichText( false );

	m_tabs->addTab( m_driverInfoWidget, tr( "GL Info" ) );
}


/*
========================
createLayout

 positions the created widgets on the client area
========================
*/
void CProgramWindow::createLayout( void )
{
	QWidget* central = new QWidget;

	QGridLayout* layout = new QGridLayout;
	layout->addWidget( m_glWidget );
	layout->addWidget( m_tabs );
	central->setLayout( layout );

	setCentralWidget( central );
}


/*
========================
init
========================
*/
bool CProgramWindow::init( void )
{
	QApplication::setOverrideCursor( Qt::WaitCursor );

	// this performs implicit OpenGL initialization in CGLWidget
	show();

	// move to top left corner of the screen
	move( 21,21 );

	// check initialization status
	if( !m_glWidget->isSuccessfullyInitialized() )
	{
		return false;
	}

	// init components
	m_scene->init();

	// initialize widgets
	m_sceneWidget->init();
	m_texture->init();
	m_driverInfoWidget->setPlainText( m_glWidget->getDriverInfoString() );

	// lock the tab size after all widgets are initialzed.
	// -> allows the GL widget to take more space
	m_tabs->setMaximumHeight( m_tabs->height() );

	//
	// init editor
	//
	m_editor = new CEditor( m_scene->getShader() );
	connect( m_editor, SIGNAL(linkProgram()), this, SLOT(linkProgram()) );
	connect( m_editor, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()) );
	connect( m_editor, SIGNAL(deactivateProgram()), this, SLOT(deactivateProgram()) );
	m_editor->init( QPoint( x() + frameGeometry().width(), y() ) );

	QApplication::restoreOverrideCursor();

	return true;
}


/*
========================
shutdown
========================
*/
void CProgramWindow::shutdown( void )
{
	// editor
	if( m_editor != NULL ) {
		disconnect( m_editor, 0, this, 0 );
		m_editor->shutdown();
		SAFE_DELETE( m_editor );
	}

	// geometry
	if( m_sceneWidget != NULL ) {
		m_sceneWidget->shutdown();
	}

	// textures
	if( m_texture != NULL ) {
		m_texture->shutdown();
	}

	// scene
	if( m_scene != NULL ) {
		m_scene->shutdown();
	}
}


/*
========================
closeEvent
========================
*/
void CProgramWindow::closeEvent( QCloseEvent* event )
{
	// ask to save work!
	if( m_editor != NULL && !m_editor->maybeSave() ) {
		event->ignore();
		return;
	}

	event->ignore();

	// set app quit flag
	aboutToQuit();
}


/*
========================
aboutToQuit

 we are going to exit now...
========================
*/
void CProgramWindow::aboutToQuit( void )
{
	qApp->exit( 0 );
}


/*
========================
deactivateProgram
========================
*/
void CProgramWindow::deactivateProgram( void )
{
	if( m_scene != NULL )
	{
		m_scene->getShader()->deactivateProgram();
	}
}


/*
========================
linkProgram
========================
*/
void CProgramWindow::linkProgram( void )
{
	if( m_scene != NULL )
	{
		// compile and link
		bool result = m_scene->getShader()->compileAndLink();

		// update state manipulation widgets
		m_uniform->updateUniformList();
		m_texture->updateSamplerList();

		// update log
		m_logging->setPlainText( m_scene->getShader()->getBuildLog() );

		// if there are any errors, switch to the log widget
		if( !result )
			m_tabs->setCurrentWidget( m_logging );
	}
}


/*
========================
render
========================
*/
void CProgramWindow::render( void )
{
	m_scene->render();
}




