//=============================================================================
/** @file		programwindow.h
 *
 * Defines the main window class.
 *
	@internal
	created:	2007-10-22
	last mod:	2008-01-11

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __QT_PROGRAMWINDOW_H_INCLUDED__
#define __QT_PROGRAMWINDOW_H_INCLUDED__

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>


// forward declarations
class IShader;
class IScene;
class CLightWidget;
class CMaterialWidget;
class CUniformWidget;
class CSceneWidget;
class CTextureWidget;
class CGLWidget;
class CEditor;


//=============================================================================
//	CProgramWindow
//=============================================================================

/** This is the applications main window.
 * It represents the entire shader editor application.
 * It contains all configuration widgets and stores the editor window object.
 */
class CProgramWindow : public QMainWindow
{
	Q_OBJECT
public:
	CProgramWindow( void ); ///< Constructs a CProgramWindow object.
	virtual ~CProgramWindow( void ); ///< Destruction.

	/** Initializes the object.
	 * It creates and shows the wodgets etc.
	 * @return Wether initalization succeeded.
	 */
	bool init( void );

	/** Cleans up the object and prepares for object destruction.
	 */
	void shutdown( void );

private slots:;
	void render( void );
	void linkProgram( void );
	void deactivateProgram( void );
	void aboutToQuit( void );

private:

	// QWidget
	void closeEvent( QCloseEvent* event );

	// construction helpers
	void createGlWidget( void );
	void createLogWidget( void );
	void createLightWidget( void );
	void createUniformWidget( void );
	void createSceneWidget( void );
	void createTextureWidget( void );
	void createLayout( void );
	void createDriverInfoWidget( void );

	// widgets
	CGLWidget*			m_glWidget;
	QTabWidget*			m_tabs; // contains config widgets
	CLightWidget*		m_lighting;
	CMaterialWidget*	m_material;
	CUniformWidget*		m_uniform;
	CSceneWidget*		m_sceneWidget;
	CTextureWidget*		m_texture;
	QTextEdit*			m_logging;
	QTextEdit*			m_driverInfoWidget; // shows info about the GL driver

	// components
	CEditor*	m_editor;
	IScene*		m_scene;
};


#endif	// __QT_PROGRAMWINDOW_H_INCLUDED__

