//=============================================================================
/** @file		editor.cpp
 *
 * Implements CEditor.
 *
	@internal
	created:	2008-01-11
	last mod:	2008-01-13

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include "application.h"
#include "editor.h"
#include "editwindow.h"
#include "shader.h"

#include <assert.h>


//=============================================================================
//	CEditor implementation
//=============================================================================

// construction
CEditor::CEditor( IShader* shader )
{
	m_shader = shader;

	// alloc pointer array.
	m_editWindows = new CBaseEditWindow* [ IShader::MAX_SHADER_TYPES ];
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
		m_editWindows[ i ] = NULL;

	m_isMDI = true;
}

CEditor::~CEditor( void )
{
	// free the pointer array.
	SAFE_DELETE_ARRAY( m_editWindows );
}


/*
========================
init
========================
*/
void CEditor::init( const QPoint & position )
{
	// assume not initialized
	assert( m_editWindows[0] == NULL );

	//
	// create the editor window(s)
	//
#if defined(__APPLE__) // make Mac users happy
	createSdiWindow( position, true );
#else
	createMdiWindow( position, true );
#endif

	// auto-compile the initial shaders.
	link();
}


/*
========================
shutdown
========================
*/
void CEditor::shutdown( void )
{
	destroyEditWindow();
}


/*
========================
maybeSave
========================
*/
bool CEditor::maybeSave( void )
{
	// ask every editor window
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL &&
			!m_editWindows[ i ]->maybeSave() )
		{
			return false;
		}
	}

	return true;
}


/*
========================
switchToMDI
========================
*/
void CEditor::switchToMDI( void )
{
	int i;
	bool hasPosition = false;
	QPoint position;
	QString fileNames[ IShader::MAX_SHADER_TYPES ];

	// already MDI
	if( m_isMDI )
		return;

	// make sure no data is lost
	if( !maybeSave() )
		return;

	//
	// collect filenames of the shaders
	//
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL )
		{
			// remember position of the first window.
			if( !hasPosition )
			{
				position = m_editWindows[i]->pos();
				hasPosition = true;
			}

			CSdiEditWindow* sdi = (CSdiEditWindow*)m_editWindows[ i ];
			fileNames[ i ] = sdi->fileName();
		}
	}

	// create the 'other' view
	destroyEditWindow();
	createMdiWindow( position, false );

	//
	// load the files that were in use before
	//
	CMdiEditWindow* mdi = (CMdiEditWindow*)m_editWindows[ 0 ];
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		mdi->loadSourceFile( i, fileNames[i] );
	}
}


/*
========================
switchToSDI
========================
*/
void CEditor::switchToSDI( void )
{
	int i;
	QPoint position;
	QString fileNames[ IShader::MAX_SHADER_TYPES ];

	// already SDI
	if( !m_isMDI )
		return;

	// make sure no data is lost
	if( !maybeSave() )
		return;

	//
	// collect filenames of the shaders
	//
	position = m_editWindows[0]->pos(); // remember position
	CMdiEditWindow* mdi = (CMdiEditWindow*)m_editWindows[ 0 ];
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		fileNames[ i ] = mdi->fileName( i );
	}

	// create the 'other' view
	destroyEditWindow();
	createSdiWindow( position, false );

	//
	// load the files that were in use before
	//
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL )
		{
			CSdiEditWindow* sdi = (CSdiEditWindow*)m_editWindows[ i ];
			sdi->loadSourceFile( fileNames[i] );
		}
	}
}


/*
========================
createMdiWindow
========================
*/
void CEditor::createMdiWindow( const QPoint & position, bool loadStdSource )
{
	m_editWindows[0] = new CMdiEditWindow( m_shader );
	setupSignals();

	if( loadStdSource )
	{
		m_editWindows[0]->loadInitialShaderSource();
	}

	m_editWindows[0]->move( position );
	m_editWindows[0]->show();

	m_isMDI = true;
}


/*
========================
createSdiWindow
========================
*/
void CEditor::createSdiWindow( const QPoint & position, bool loadStdSource )
{
	int i;

	// create editor windows
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_shader->isShaderTypeAvailable( i ) )
		{
			m_editWindows[ i ] = new CSdiEditWindow( m_shader, i );
		}
	}

	setupSignals();

	// initialize editor windows
	for( i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] == NULL )
			continue;

		if( loadStdSource )
		{
			m_editWindows[i]->loadInitialShaderSource();
		}

		m_editWindows[i]->move( position + QPoint( i * 20, 0 ) );
		m_editWindows[i]->show();
	}

	m_isMDI = false;
}


/*
========================
destroyEditWindow
========================
*/
void CEditor::destroyEditWindow( void )
{
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL )
		{
			disconnect( m_editWindows[ i ], 0, this, 0 );
			SAFE_DELETE( m_editWindows[ i ] );
		}
	}
}


/*
========================
setupSignals
========================
*/
void CEditor::setupSignals( void )
{
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL )
		{
			connect( m_editWindows[i], SIGNAL(linkProgram()),       this, SLOT(link()) );
			connect( m_editWindows[i], SIGNAL(deactivateProgram()), this, SLOT(shouldDeactivateProgram()) );
			connect( m_editWindows[i], SIGNAL(aboutToQuit()),       this, SLOT(gotQuitSignal()) );
			connect( m_editWindows[i], SIGNAL(requestMdiMode()),    this, SLOT(switchToMDI()) );
			connect( m_editWindows[i], SIGNAL(requestSdiMode()),    this, SLOT(switchToSDI()) );
		}
	}
}


/*
========================
link
========================
*/
void CEditor::link( void )
{
	// upload shader source code.
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL )
		{
			m_editWindows[ i ]->uploadShaderSource( m_shader );
		}
	}

	// request a link operation.
	emit linkProgram();
}


/*
========================
shouldDeactivateProgram
========================
*/
void CEditor::shouldDeactivateProgram( void )
{
	emit deactivateProgram();
}


/*
========================
gotQuitSignal
========================
*/
void CEditor::gotQuitSignal( void )
{
	// make sure all editors can save their work
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editWindows[ i ] != NULL &&
			!m_editWindows[ i ]->maybeSave() )
		{
			return;
		}
	}

	emit aboutToQuit();
}


