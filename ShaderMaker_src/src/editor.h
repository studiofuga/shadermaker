//=============================================================================
/** @file		editor.h
 *
 * Defines the top level source code editor class.
 *
	@internal
	created:	2008-01-11
	last mod:	2008-01-12

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __EDITOR_H_INCLUDED__
#define __EDITOR_H_INCLUDED__

#include <QtCore/QObject>
#include <QtCore/QPoint>


// forward declarations
class IShader;
class CBaseEditWindow;


//=============================================================================
//	CEditor
//=============================================================================

/** The main source code editing component.
 * This class is the top level interface to the GLSL source code
 * editor component. It is bound to an IShader object which 
 * must be already initilized when this constructor is called.
 * The created text editing widgets are based on the state
 * in that shader object. The source code written with
 * this editor will be stored in that shader object, too.
 */
class CEditor : public QObject
{
	Q_OBJECT
public:
	/** Constructor.
	 * @param shader IShader object this editor is bound to.
	 */
	CEditor( IShader* shader );
	virtual ~CEditor( void ); ///< Destruction.

	/** Initializes and shows the editor.
	 * @param position Screen coordinates of the top left corner
	 * of the editor window to appear.
	 */
	void init( const QPoint & position );

	/** Cleans up state and prepares for destruction.
	 */
	void shutdown( void );

	/** Asks the user wether to save modified documents.
	 * This call asks the user for each modified shader source code.
	 * @return Returns true if the calling operation should coninue, otherwise false.
	 *			Simple: True means 'no data will get lost'.
	 */
	bool maybeSave( void );

signals:
	/** The user requested to compile link the current shader.
	 * @pre
	 *	When this signal is emitted, the editor has already stored the required
	 *  source code in the bound IShader object.
	 */
	void linkProgram( void );

	/** The current shader should be deactivated.
	 * This happens, when the user opens an existing or creates a new shader.
	 */
	void deactivateProgram( void );

	/** Tells the application that the user wants to quit the application.
	 */
	void aboutToQuit( void ); // the editor is ready to quit the app.

private slots:;

	// these are forwarded from the editor windows
	void link();
	void shouldDeactivateProgram( void );
	void gotQuitSignal( void );

	// switching between SDI and MDI windows.
	void switchToSDI( void );
	void switchToMDI( void );

private:

	// changing the editor window
	void createMdiWindow( const QPoint & position, bool loadStdSource );
	void createSdiWindow( const QPoint & position, bool loadStdSource );
	void destroyEditWindow( void );
	void setupSignals( void );

	// the shader object
	IShader* m_shader;

	// Wether the editor is in MDI mode.
	// The initial state is true.
	bool m_isMDI;

	// Ihere is a single window for every shader type.
	// If the editor is in single-window mode, 
	// then only m_editWindows[0] is used, the others are NULL.
	CBaseEditWindow** m_editWindows; // [ IShader::MAX_SHADER_TYPES ]
};

#endif // __EDITOR_H_INCLUDED__

