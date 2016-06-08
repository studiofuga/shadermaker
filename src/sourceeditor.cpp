//=============================================================================
/** @file		sourceeditor.cpp
 *
 * Implements CSourceEdit.
 *
	@internal
	created:	2008-01-03
	last mod:	2008-01-12

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

	GZ: evtl QCodeEdit stattdessen verwenden (hat mehr Features)
=============================================================================*/

#include <QtCore/QTextStream>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>

#include "application.h"
#include "sourceeditor.h"
#include "shader.h"
#include "stdshader.h"


//=============================================================================
//	CSourceEdit implementation
//=============================================================================

// construction
CSourceEdit::CSourceEdit( int shaderType )
{
	// member initialization
	m_shaderType = shaderType;
	m_fileName = QString( "" );
	m_highlighter = NULL;

	// setup font
	QFont f( font() );
	f.setFamily( tr( CONFIG_EDITOR_FONT_NAME ) );
	f.setPointSize( CONFIG_EDITOR_FONT_SIZE );
	f.setFixedPitch( true );
	setFont( f );

	// turn off dynamic line wrap
	setLineWrapMode( NoWrap );

	// set tab sizes...
	char text[ CONFIG_TAB_SIZE+1 ];
	memset( text, ' ', CONFIG_TAB_SIZE );
	text[ CONFIG_TAB_SIZE ] = '\0';
	setTabStopWidth( fontMetrics().width( text ) );

	// syntax highlighting
	createSyntaxHighlighter();

	// notify parent
	connect( document(), SIGNAL(contentsChanged()), this, SLOT(documentContentsChanged()) );
}


/*
========================
loadInitialShaderSource
========================
*/
void CSourceEdit::loadInitialShaderSource( void )
{
	// initial shader source is not assigned to a file.
	setFileName( "" );
	setPlainText( "" );

	//
	// set initial fragment shaders
	//
	switch( m_shaderType )
	{
	case IShader::TYPE_VERTEX:
		setPlainText( INITIAL_VERTEX_SHADER_SOURCE );
		break;

	case IShader::TYPE_GEOMETRY:
		setPlainText( INITIAL_GEOMETRY_SHADER_SOURCE );
		break;

	case IShader::TYPE_FRAGMENT:
		setPlainText( INITIAL_FRAGMENT_SHADER_SOURCE );
		break;
	}

	// clear the modified flag
	document()->setModified( false );
}


/*
========================
documentContentsChanged

 this is only used to reduce the amount connections between objects.
========================
*/
void CSourceEdit::documentContentsChanged( void )
{
	// notify parent
	emit updateMainWindow();
}


/*
========================
newFile
========================
*/
void CSourceEdit::newFile( void )
{
	if( !maybeSave() )
		return;

	// set new state
	loadInitialShaderSource();
	setFocus( Qt::OtherFocusReason );

	emit updateMainWindow();
	emit shaderChangedCompletely();
}


/*
========================
open
========================
*/
void CSourceEdit::open( void )
{
	if( !maybeSave() )
		return;

	// setup initial directory
	QString initialDir = CONFIG_SHADER_DIRECTORY;
	if( m_fileName.length() > 0 )
		initialDir = m_fileName;

	//
	// ask for the file to open
	//
	QString fileName = QFileDialog::getOpenFileName(
									parentWidget(), QString(),
									initialDir,
									fileTypeFilter() );
	setFocus( Qt::OtherFocusReason );
	if( !fileName.isEmpty() )
	{
		loadFile( fileName );
	}
}


/*
========================
save
========================
*/
bool CSourceEdit::save( void )
{
	if( m_fileName.isEmpty() )
		return saveAs();

	return saveFile( m_fileName );
}


/*
========================
saveAs
========================
*/
bool CSourceEdit::saveAs( void )
{
	// setup initial directory
	QString initialDir = CONFIG_SHADER_DIRECTORY;
	if( m_fileName.length() > 0 )
		initialDir = m_fileName;

	// bring up save dialog
	QString fileName = QFileDialog::getSaveFileName( 
									parentWidget(), QString(),
									initialDir,
									fileTypeFilter() );
	setFocus( Qt::OtherFocusReason );
	if( fileName.isEmpty() )
		return false;

	return saveFile( fileName );
}


/*
========================
loadFile
========================
*/
void CSourceEdit::loadFile( const QString & fileName )
{
	QFile file( fileName );

	if( !file.open( QFile::ReadOnly | QFile::Text ) )
	{
		QMessageBox::warning( parentWidget(),
							  tr( CONFIG_STRING_APPLICATION_TITLE ),
							  tr( "Cannot read file %1:\n%2." )
							  .arg(fileName)
							  .arg(file.errorString()));
		return;
	}

	QTextStream in( &file );
	QApplication::setOverrideCursor( Qt::WaitCursor );
	QString text = in.readAll();

	// set file name first, so the contentsChanged() signal will be up to date
	setFileName( fileName );
	setPlainText( text );

	QApplication::restoreOverrideCursor();

	emit updateMainWindow();
	emit shaderChangedCompletely();
}


/*
========================
saveFile
========================
*/
bool CSourceEdit::saveFile( const QString & fileName )
{
	QFile file( fileName );

	if( !file.open( QFile::WriteOnly | QFile::Text ) )
	{
		QMessageBox::warning( parentWidget(), 
							  tr( CONFIG_STRING_APPLICATION_TITLE ),
							  tr( "Cannot write file %1:\n%2." )
                              .arg(fileName)
                              .arg(file.errorString()));
         return false;
	}

	QTextStream out( &file );
	QApplication::setOverrideCursor(Qt::WaitCursor);
	out << toPlainText();
	QApplication::restoreOverrideCursor();

	setFileName( fileName );
	document()->setModified( false );
	emit updateMainWindow();

	return true;
}


/*
========================
maybeSave

 returns true, if the interrupted operation should continue
========================
*/
bool CSourceEdit::maybeSave( void )
{
	// no need to save it.
	if( !document()->isModified() )
		return true;

	// query good file name
	QString name = fileName();
	if( name.isEmpty() )
		name = QString( CONFIG_STRING_FILE_UNTITLED );

	// setup text and title
	QString title( CONFIG_STRING_SHOULD_SAVE_FILE_TITLE );
	QString text = QString( CONFIG_STRING_SHOULD_SAVE_FILE ).
					arg( IShader::getShaderTypeName( m_shaderType ) ).
					arg( name );

	// display the message box
	int ret = QMessageBox::question( parentWidget(), title, text,
		QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
		QMessageBox::Cancel );

	switch( ret )
	{
	case QMessageBox::Save:
		return save();
		break;

	case QMessageBox::Discard:
		return true;
		break;
	}

	// default: cancel
	return false;
}


/*
========================
setFileName
========================
*/
void CSourceEdit::setFileName( const QString & fileName )
{
	m_fileName = fileName;
}


/*
========================
fileName
========================
*/
QString CSourceEdit::fileName( void ) const
{
	return m_fileName;
}


/*
========================
shaderType
========================
*/
int CSourceEdit::shaderType( void ) const
{
	return m_shaderType;
}


/*
========================
fileTypeFilter
========================
*/
QString CSourceEdit::fileTypeFilter( void )
{
	QString specific;
	QString all( "All Files (*)" );

	switch( m_shaderType )
	{
	case IShader::TYPE_VERTEX:   specific = "Vertex Shaders (*.vert);;"; break;
	case IShader::TYPE_GEOMETRY: specific = "Geometry Shaders (*.geom);;"; break;
	case IShader::TYPE_FRAGMENT: specific = "Fragment Shaders (*.frag);;"; break;
	}

	return ( specific + all );
}


