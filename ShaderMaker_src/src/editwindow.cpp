//=============================================================================
/** @file		editwindow.cpp
 *
 * Implements the text editor window classes.
 *
	@internal
	created:	2007-10-24
	last mod:	2008-02-11

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QMenuBar>
#include <QtGui/QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QGridLayout>
#include <QInputDialog>

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QIntValidator>

#include "application.h"
#include "editwindow.h"
#include "sourceeditor.h"
#include "shader.h"

//=============================================================================
//	CBaseEditWindow implementation
//=============================================================================

// construction
CBaseEditWindow::CBaseEditWindow( void )
{
	// buttons
	m_btnLink = new QPushButton( "Compile and Link (F5)" );
	connect( m_btnLink, SIGNAL( clicked() ), this, SLOT( link() ) );

	m_lineNumber = new QLabel(this);
	m_lineNumber->setAlignment(Qt::AlignRight);

	// check boxes.
	m_chkAttach = new QCheckBox( "Attach to program" );

	// make sure no dummy text appears
	setWindowTitle( CONFIG_STRING_APPLICATION_TITLE );
}

CBaseEditWindow::~CBaseEditWindow( void )
{
}


/*
========================
newFile
========================
*/
void CBaseEditWindow::newFile( void )
{
	CSourceEdit* doc = activeDocument();
	if( doc != NULL )
		doc->newFile();
}


/*
========================
open
========================
*/
void CBaseEditWindow::open( void )
{
	CSourceEdit* doc = activeDocument();
	if( doc != NULL )
		doc->open();
}


/*
========================
save
========================
*/
bool CBaseEditWindow::save( void )
{
	CSourceEdit* doc = activeDocument();
	if( doc != NULL )
		return doc->save();

	return false;
}


/*
========================
saveAs
========================
*/
bool CBaseEditWindow::saveAs( void )
{
	CSourceEdit* doc = activeDocument();
	if( doc != NULL )
		return doc->saveAs();

	return false;
}


/*
========================
about
========================
*/
void CBaseEditWindow::about( void )
{
	QMessageBox::about( this, CONFIG_STRING_APPLICATION_TITLE,
							  CONFIG_STRING_ABOUT_BOX_TEXT );
}


/*
========================
aboutQt
========================
*/
void CBaseEditWindow::aboutQt( void )
{
	QMessageBox::aboutQt( this, CONFIG_STRING_APPLICATION_TITLE );
}


/*
========================
undo/redo/copy/cut/paste
========================
*/
void CBaseEditWindow::undo( void )
{
	QTextEdit* doc = activeDocument();
	if( doc != NULL ) { doc->undo(); }
}
void CBaseEditWindow::redo( void )
{
	QTextEdit* doc = activeDocument();
	if( doc != NULL ) { doc->redo(); }
}
void CBaseEditWindow::copy( void )
{
	QTextEdit* doc = activeDocument();
	if( doc != NULL ) { doc->copy(); }
}
void CBaseEditWindow::cut( void )
{
	QTextEdit* doc = activeDocument();
	if( doc != NULL ) { doc->cut(); }
}
void CBaseEditWindow::paste( void )
{
	QTextEdit* doc = activeDocument();
	if( doc != NULL ) { doc->paste(); }
}


/*
========================
link
========================
*/
void CBaseEditWindow::link( void )
{
	// compile and link
	emit linkProgram();
}


/*
========================
contentsChanged
========================
*/
void CBaseEditWindow::contentsChanged( void )
{
	updateWindowTitle();
}


/*
========================
shaderFileChanged
========================
*/
void CBaseEditWindow::shaderFileChanged( void )
{
	// there is a new shader. deactivate the old one.
	emit deactivateProgram();
}


/*
========================
updateWindowTitle
========================
*/
void CBaseEditWindow::updateWindowTitle( void )
{
	CSourceEdit* doc = activeDocument();
	if( doc != NULL )
	{
		// get the base file name
		QString title = extractFileNameFromPath( doc->fileName() );
		if( title.length() == 0 ) // source not assigned to a file
			title = QString( CONFIG_STRING_FILE_UNTITLED );

		// add 'modified' flag
		if( doc->document()->isModified() ) {
			title += tr( "*" );
		}

		// lookup the shader type name
		QString shType = QString( " (%1) " ).
			arg( IShader::getShaderTypeName( doc->shaderType() ) );

		// add app title
		title += shType; // show shader type in the window title
		title += QString( " - " CONFIG_STRING_APPLICATION_TITLE );

		setWindowTitle( title );
	}
	else // no active document
	{
		setWindowTitle( CONFIG_STRING_APPLICATION_TITLE );
	}
}



/*
========================
createActions
========================
*/
void CBaseEditWindow::createActions( IShader* )
{
	//
	// files
	//
	m_actNew = new QAction( QIcon(":/images/new.png"), tr( "&New" ), this );
	m_actNew->setShortcut( tr( "Ctrl+N" ) );
	connect( m_actNew, SIGNAL(triggered() ), this, SLOT(newFile()) );

	m_actOpen = new QAction( QIcon(":/images/open.png"), tr( "&Open..." ), this );
	m_actOpen->setShortcut( tr( "Ctrl+O" ) );
	connect( m_actOpen, SIGNAL(triggered()), this, SLOT(open()) );

	m_actSave = new QAction( QIcon(":/images/save.png"), tr( "&Save" ), this );
	m_actSave->setShortcut( tr( "Ctrl+S" ) );
	connect( m_actSave, SIGNAL(triggered()), this, SLOT(save()) );

	m_actSaveAs = new QAction( tr( "Save &As" ), this );
	connect( m_actSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()) );

	m_actQuit = new QAction( QIcon(":/images/exit.png"), tr( "E&xit" ), this );
	m_actQuit->setShortcut( tr( "Ctrl+Q" ) );
	connect( m_actQuit, SIGNAL(triggered()), this, SLOT(close()) );

	//
	// edit
	//
	m_actUndo = new QAction( QIcon(":/images/undo.png"), tr( "&Undo" ), this );
	m_actUndo->setShortcut( tr( "Ctrl+Z" ) );
	connect( m_actUndo, SIGNAL(triggered()), this, SLOT(undo()) );

	m_actRedo = new QAction( QIcon(":/images/redo.png"), tr( "&Redo" ), this );
	m_actRedo->setShortcut( tr( "Ctrl+Shift+Z" ) );
	connect( m_actRedo, SIGNAL(triggered()), this, SLOT(redo()) );

	m_actCopy = new QAction( QIcon(":/images/copy.png"), tr( "&Copy" ), this );
	m_actCopy->setShortcut( tr( "Ctrl+C" ) );
	connect( m_actCopy, SIGNAL(triggered()), this, SLOT(copy()) );

	m_actCut = new QAction( QIcon(":/images/cut.png"), tr( "Cu&t" ), this );
	m_actCut->setShortcut( tr( "Ctrl+X" ) );
	connect( m_actCut, SIGNAL(triggered()), this, SLOT(cut()) );

	m_actPaste = new QAction( QIcon(":/images/paste.png"), tr( "&Paste" ), this );
	m_actPaste->setShortcut( tr( "Ctrl+V" ) );
	connect( m_actPaste, SIGNAL(triggered()), this, SLOT(paste()) );

	//
	// shaders
	//
	m_actLink = new QAction( tr( "&Link" ), this );
	m_actLink->setShortcut( tr( "F5" ) );
	connect( m_actLink, SIGNAL(triggered()), this, SLOT(link()) );

	//
	// help
	//
	m_actAbout = new QAction( tr( "&About" ), this );
	connect( m_actAbout, SIGNAL(triggered()), this, SLOT(about()) );

	m_actAboutQt = new QAction( QIcon(":/images/qt.png"), tr( "About &Qt" ), this );
	connect( m_actAboutQt, SIGNAL(triggered()), this, SLOT(aboutQt()) );
}


/*
========================
createMenus
========================
*/
void CBaseEditWindow::createMenus( IShader* )
{
	// files 
	m_menuFile = menuBar()->addMenu( tr( "&File" ) );
	m_menuFile->addAction( m_actNew );
	m_menuFile->addAction( m_actOpen );
	m_menuFile->addAction( m_actSave );
	m_menuFile->addAction( m_actSaveAs );
	m_menuFile->addSeparator();
	m_menuFile->addAction( m_actQuit );

	// edit
	m_menuEdit = menuBar()->addMenu( "&Edit" );
	m_menuEdit->addAction( m_actUndo );
	m_menuEdit->addAction( m_actRedo );
	m_menuEdit->addSeparator();
	m_menuEdit->addAction( m_actCopy );
	m_menuEdit->addAction( m_actCut  );
	m_menuEdit->addAction( m_actPaste );

	// shader
	m_menuShader = menuBar()->addMenu( tr( "&Shader" ) );
	m_menuShader->addAction( m_actLink );

	// view
	m_menuView = menuBar()->addMenu( tr( "&View" ) );

	// help
	m_menuHelp = menuBar()->addMenu( tr( "&?" ) );
	m_menuHelp->addAction( m_actAbout );
	m_menuHelp->addAction( m_actAboutQt );
}


/*
========================
closeEvent
========================
*/
void CBaseEditWindow::closeEvent( QCloseEvent* event )
{
	// do NOT close!
	event->ignore();

	// forward to the owner.
	emit aboutToQuit();
}


//=============================================================================
//	CSdiEditWindow implementation
//=============================================================================

// construction
CSdiEditWindow::CSdiEditWindow( IShader* shader, int shaderType )
{
	m_document = new CSourceEdit( shaderType );

	// don't attach geometry shaders be default
	// -> confuses the novice
	m_attachToShader = ( shaderType == IShader::TYPE_GEOMETRY ) ? false : true;

	m_chkAttach->setCheckState( m_attachToShader ? Qt::Checked : Qt::Unchecked );

	// setup signals
	connect( m_document, SIGNAL(updateMainWindow()), this, SLOT(contentsChanged()) );
	connect( m_document, SIGNAL(shaderChangedCompletely()), this, SLOT(shaderFileChanged()) );
	connect( m_document, SIGNAL(cursorPositionChanged()), this, SLOT(positionChanged()) );
	connect( m_chkAttach, SIGNAL(stateChanged(int)), this, SLOT(checkAttachToShader(int)) );

	createActions( shader );
	createMenus  ( shader );

	// setup layout
	QWidget* central = new QWidget;
	QGridLayout* layout = new QGridLayout;
	layout->addWidget( m_document,  0, 0, 1, 3 );
	layout->addWidget( m_chkAttach, 1, 0, 1, 1 );
	layout->addWidget( m_btnLink,   1, 1, 1, 1 );
	layout->addWidget( m_lineNumber,1, 2, 1, 1 );
	central->setLayout( layout );

	setCentralWidget( central );
}


/*
========================
positionChanged
========================
*/
void CSdiEditWindow::positionChanged( void ) {
	QTextCursor cursor = m_document->textCursor();
	int ln = cursor.blockNumber() + 1;
	int col = cursor.columnNumber() + 1;
	m_lineNumber->setText( "Ln: " + QString::number(ln) + " | Col: " + QString::number(col) );
}


/*
========================
maybeSave
========================
*/
bool CSdiEditWindow::maybeSave( void )
{
	return m_document->maybeSave();
}


/*
========================
loadInitialShaderSource
========================
*/
void CSdiEditWindow::loadInitialShaderSource( void )
{
	m_document->loadInitialShaderSource();
	updateWindowTitle();
}


/*
========================
uploadShaderSource
========================
*/
void CSdiEditWindow::uploadShaderSource( IShader* shader )
{
	if( m_attachToShader )
	{
		shader->setShaderSource( m_document->shaderType(), 
								 m_document->document()->toPlainText() );
	}
	else // disabled by the user
	{
		shader->setShaderSource( m_document->shaderType(), QString( "" ) );
	}
}


/*
========================
loadSourceFile
========================
*/
void CSdiEditWindow::loadSourceFile( const QString & fileName )
{
	// if there is text in the string
	if( !fileName.isEmpty() )
	{
		m_document->loadFile( fileName );
	}
	else
	{
		m_document->loadInitialShaderSource();
	}

	updateWindowTitle();
}


/*
========================
fileName
========================
*/
QString CSdiEditWindow::fileName( void ) const
{
	return m_document->fileName();
}


/*
========================
createActions
========================
*/
void CSdiEditWindow::createActions( IShader* shader )
{
	CBaseEditWindow::createActions( shader );

	m_actToMDI = new QAction( tr( "Switch to MDI view" ), this );
	connect( m_actToMDI, SIGNAL(triggered()), this, SLOT(requestMdiModeSlot()) );
}


/*
========================
createMenus
========================
*/
void CSdiEditWindow::createMenus( IShader* shader )
{
	CBaseEditWindow::createMenus( shader );

	//
	// view
	//
	m_menuView->addAction( m_actToMDI );
}


/*
========================
checkAttachToShader
========================
*/
void CSdiEditWindow::checkAttachToShader( int checkState )
{
	m_attachToShader = ( checkState != Qt::Unchecked );
}


/*
========================
requestMdiModeSlot
========================
*/
void CSdiEditWindow::requestMdiModeSlot( void )
{
	emit requestMdiMode();
}


//=============================================================================
//	CMdiEditWindow implementation
//=============================================================================

// construction
CMdiEditWindow::CMdiEditWindow( IShader* shader )
{
	m_editors = new CSourceEdit*	[ IShader::MAX_SHADER_TYPES ];
	m_actFocusEditor = new QAction*	[ IShader::MAX_SHADER_TYPES ];
	m_attachToShader = new bool		[ IShader::MAX_SHADER_TYPES ];
	m_signalMapper = new QSignalMapper( this );
	createActions( shader );
	createMenus  ( shader );
	createTabs   ( shader );

	// setup layout
	QWidget* central = new QWidget;
	QGridLayout* layout = new QGridLayout;
	layout->addWidget( m_tabs,      0, 0, 1, 3 );
	layout->addWidget( m_chkAttach, 1, 0, 1, 1 );
	layout->addWidget( m_btnLink,   1, 1, 1, 1 );
	layout->addWidget( m_lineNumber,1, 2, 1, 1 );
	central->setLayout( layout );
	setCentralWidget( central );

	connect( m_chkAttach, SIGNAL(stateChanged(int)), this, SLOT(checkAttachToShader(int)) );

	// trigger state update
	activeEditorChanged( m_tabs->currentIndex() );
}

CMdiEditWindow::~CMdiEditWindow( void )
{
	SAFE_DELETE_ARRAY( m_editors );
	SAFE_DELETE_ARRAY( m_actFocusEditor );
	SAFE_DELETE_ARRAY( m_attachToShader );
}


/*
========================
maybeSave

 returns true, if the caller should continue.
========================
*/
bool CMdiEditWindow::maybeSave( void )
{
	// loop through all sources
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editors[ i ] != NULL &&
			!m_editors[ i ]->maybeSave() )
		{
			return false;
		}
	}

	return true;
}


/*
========================
loadInitialShaderSource
========================
*/
void CMdiEditWindow::loadInitialShaderSource( void )
{
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editors[ i ] != NULL )
		{
			m_editors[ i ]->loadInitialShaderSource();
		}
	}

	updateWindowTitle();
}


/*
========================
loadSourceFile
========================
*/
void CMdiEditWindow::loadSourceFile( int shaderType, const QString & fileName )
{
	if( shaderType >= 0 && shaderType < IShader::MAX_SHADER_TYPES &&
		m_editors[ shaderType ] != NULL )
	{
		// if there is text in the string
		if( !fileName.isEmpty() )
		{
			m_editors[ shaderType ]->loadFile( fileName );
		}
		else
		{
			m_editors[ shaderType ]->loadInitialShaderSource();
		}
	}

	updateWindowTitle();
}


/*
========================
fileName
========================
*/
QString CMdiEditWindow::fileName( int shaderType )
{
	if( shaderType >= 0 && shaderType < IShader::MAX_SHADER_TYPES &&
		m_editors[ shaderType ] != NULL )
	{
		return m_editors[ shaderType ]->fileName();
	}

	return QString( "" );
}


/*
========================
uploadShaderSource
========================
*/
void CMdiEditWindow::uploadShaderSource( IShader* shader )
{
	// read out all editors
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		// not available
		if( m_editors[ i ] == NULL )
			continue;

		if( m_attachToShader[ i ] )
		{
			shader->setShaderSource( i, m_editors[ i ]->document()->toPlainText() );
		}
		else // user diabled this shader type
		{
			shader->setShaderSource( i, QString( "" ) );
		}
	}
}


/*
========================
createActions
========================
*/
void CMdiEditWindow::createActions( IShader* shader )
{
	CBaseEditWindow::createActions( shader );

	//
	// view
	//
	m_actToSDI = new QAction( tr( "Switch to SDI view" ), this );
	connect( m_actToSDI, SIGNAL(triggered()), this, SLOT(requestSdiModeSlot()) );

	//
	// shaders
	//
	m_actNextShader = new QAction( tr( "Next Shader" ), this );
	m_actNextShader->setShortcut( tr( "Alt+Right" ) );
	connect( m_actNextShader, SIGNAL(triggered()), this, SLOT(nextShaderTab()) );

	m_actPrevShader = new QAction( tr( "Previous Shader" ), this );
	m_actPrevShader->setShortcut( tr( "Alt+Left" ) );
	connect( m_actPrevShader, SIGNAL(triggered()), this, SLOT(prevShaderTab()) );

	//
	// changing the active tab
	//
	for( int i = 0, j = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		m_actFocusEditor[ i ] = NULL;
		if( shader->isShaderTypeAvailable( i ) )
		{
			QString name = IShader::getShaderTypeName(i);
			QString shortcut = tr( "F%1" ).arg( i+1 );

			m_actFocusEditor[ i ] = new QAction( name, this );
			m_actFocusEditor[ i ]->setShortcut( shortcut );

			connect( m_actFocusEditor[ i ], SIGNAL(triggered()),
					 m_signalMapper, SLOT(map()) );
			m_signalMapper->setMapping( m_actFocusEditor[ i ], j );
			j++;
		}
	}
}


/*
========================
createMenus
========================
*/
void CMdiEditWindow::createMenus( IShader* shader )
{
	CBaseEditWindow::createMenus( shader );

	//
	// view
	//
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_actFocusEditor[ i ] != NULL )
			m_menuView->addAction( m_actFocusEditor[ i ] );
	}
	m_menuView->addSeparator();
	m_menuView->addAction( m_actNextShader );
	m_menuView->addAction( m_actPrevShader );

	m_menuView->addSeparator();
	m_menuView->addAction( m_actToSDI );
}


/*
========================
createTabs
========================
*/
void CMdiEditWindow::createTabs( IShader* shader )
{
	m_tabs = new QTabWidget();

	// add tabs
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		m_editors[ i ] = NULL;

		m_attachToShader[ i ] = false;

		// if available
		if( shader->isShaderTypeAvailable( i ) )
		{
			// don't attach geometry shaders be default
			// -> confuses the novice
			if( i != IShader::TYPE_GEOMETRY ) {
				m_attachToShader[ i ] = true;
			}

			m_editors[ i ] = new CSourceEdit( i );
			m_tabs->addTab( m_editors[ i ], IShader::getShaderTypeName( i ) );
			connect( m_editors[i], SIGNAL(updateMainWindow()), this, SLOT(contentsChanged()) );
			connect( m_editors[i], SIGNAL(shaderChangedCompletely()), this, SLOT(shaderFileChanged()) );
			connect( m_editors[i], SIGNAL(cursorPositionChanged()), this, SLOT(positionChanged()) );
		}
	}

	connect( m_tabs, SIGNAL(currentChanged(int)), this, SLOT(activeEditorChanged(int)) );
	connect( m_tabs, SIGNAL(currentChanged(int)), this, SLOT(positionChanged()) );
	connect( m_signalMapper, SIGNAL(mapped(int)), m_tabs, SLOT(setCurrentIndex(int)) );
}


/*
========================
positionChanged
========================
*/
void CMdiEditWindow::positionChanged( void ) {
	QTextCursor cursor = m_editors[tabToShader(m_tabs->currentIndex())]->textCursor();
	int ln = cursor.blockNumber() + 1;
	int col = cursor.columnNumber() + 1;
	m_lineNumber->setText( "Ln: " + QString::number(ln) + " | Col: " + QString::number(col) );
}


/*
========================
activeDocument
========================
*/
CSourceEdit* CMdiEditWindow::activeDocument( void )
{
	QWidget* widget = m_tabs->widget( m_tabs->currentIndex() );

	// do not type cast. does not hurt, since there are only 3 elements...
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editors[ i ] != NULL && 
			m_editors[ i ] == widget )
		{
			return m_editors[ i ];
		}
	}

	return NULL;
}


/*
========================
tabToShader
========================
*/
int CMdiEditWindow::tabToShader( int tabIndex )
{
	QWidget* widget = m_tabs->widget( tabIndex );

	// look up the widget... it only take 3 loops...
	for( int i = 0 ; i < IShader::MAX_SHADER_TYPES ; i++ )
	{
		if( m_editors[ i ] != NULL && 
			m_editors[ i ] == widget )
		{
			return i;
		}
	}

	// not found...
	return -1;
}


/*
========================
nextShaderTab
========================
*/
void CMdiEditWindow::nextShaderTab( void )
{
	if( m_tabs->currentIndex() < m_tabs->count()-1 )
		m_tabs->setCurrentIndex( m_tabs->currentIndex() + 1 );
}


/*
========================
prevShaderTab
========================
*/
void CMdiEditWindow::prevShaderTab( void )
{
	if( m_tabs->currentIndex() > 0 )
		m_tabs->setCurrentIndex( m_tabs->currentIndex() - 1 );
}


/*
========================
activeEditorChanged

 idx: tab-index
========================
*/
void CMdiEditWindow::activeEditorChanged( int idx )
{
	updateWindowTitle();

	idx = tabToShader( idx );

	// look up attach flag
	bool attach = false;
	if( idx >= 0 && idx < IShader::MAX_SHADER_TYPES )
		attach = m_attachToShader[ idx ];

	// set new state
	m_chkAttach->setCheckState( attach ? Qt::Checked : Qt::Unchecked );
}


/*
========================
checkAttachToShader
========================
*/
void CMdiEditWindow::checkAttachToShader( int checkState )
{
	// store check state for the current shader
	int idx = tabToShader( m_tabs->currentIndex() );
	if( idx >= 0 && idx < IShader::MAX_SHADER_TYPES )
		m_attachToShader[ idx ] = ( checkState != Qt::Unchecked );
}


/*
========================
requestSdiModeSlot
========================
*/
void CMdiEditWindow::requestSdiModeSlot( void )
{
	emit requestSdiMode();
}



