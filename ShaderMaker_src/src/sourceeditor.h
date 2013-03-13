//=============================================================================
/** @file		sourceeditor.h
 *
 * Defines the GLSL source code editing widget.
 *
	@internal
	created:	2008-01-03
	last mod:	2008-01-12

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __SOURCEEDITOR_H_INCLUDED__
#define __SOURCEEDITOR_H_INCLUDED__

#include <QTextEdit>
#include <QtGui/QSyntaxHighlighter>


//=============================================================================
//	CSourceEdit - text editor for GLSL source code.
//=============================================================================

/** This is a GLSL source code editing widget.
 * It is simply an improved QTextEdit with some additional functionality
 * like open/save/new file operations and syntax highlighting.
 * This widget knows about the shader type it is editing. This allows automatic
 * selection of filename filters in dialogs.
 * The shader type must be set during object construction.
 */
class CSourceEdit : public QTextEdit
{
	Q_OBJECT
public:
	/** Constructs a CSourceEdit object.
	 * A shader type must be specified that controls some aspects of the editor,
	 * like filters in open/save dialogs.
	 * @param shaderType Shader type from IShader.
	 */
	CSourceEdit( int shaderType );

	/** Load the initial source code for the assigned shader type.
	 * The source code loaded is based on the shaderType argument passed to the constructor.
	 */
	void loadInitialShaderSource( void );

	/** Returns the shader type assigned to this source editor.
	 */
	int shaderType( void ) const;

	/** Clears the content to an empty string.
	 * Pops up a 'save changes' dialog if necessary.
	 */
	void newFile( void );

	/** Asks the user to open a file.
	 * Pops up a 'save changes' dialog if necessary.
	 */
	void open( void );

	/** Saves the currently opened file.
	 * If no filename is stored, the uses is asked for a file name.
	 */
	bool save( void );

	/** Asks the user for a file name and saves the content in that file.
	 */
	bool saveAs( void );

	/** Asks the user to save changes, if necessary.
	 * @return True if the data is saved and the calling operation can be continued.
	 *         False if the calling operation should be aborded.
	 */
	bool maybeSave( void );

	/** Returns the filename of the currently opened file.
	 * An empty string indicates that no file name is assigned to the current content.
	 */
	QString fileName( void ) const;

	/** Tells the widget to directly load a named file.
	 * @param fileName Name of the file to load.
	 */
	void loadFile( const QString & fileName );

signals:;
	/** Emitted every time the owner of this widget needs to update.
	 * For example, this happens when the documents modified flag changed.
	 */
	void updateMainWindow( void );

	/** Indicates that the used opened an existing or created a new shader.
	 * It is emitted every time a document was successfully loaded from disk.
	 */
	void shaderChangedCompletely( void );

private slots:;
	void documentContentsChanged( void );

private:
	bool saveFile( const QString & fileName );
	void setFileName( const QString & fileName );
	QString fileTypeFilter( void );

	// auto sizing
	QSize minimumSizeHint( void ) const { return QSize( 200, 100 ); }
	QSize        sizeHint( void ) const { return QSize( 500, 400 ); }


	// creates the syntax highlighter for GLSL
	// and assigns it to this CSourceEdit.
	void createSyntaxHighlighter( void );
	QSyntaxHighlighter* m_highlighter;

	int		m_shaderType;
	QString	m_fileName; // empty string == untitled document
};

#endif	// __SOURCEEDITOR_H_INCLUDED__

