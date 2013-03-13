//=============================================================================
/** @file		scenewidget.h
 *
 * Defines a scene configuration widget.
 *
	@internal
	created:	2007-11-24
	last mod:	2008-01-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __SCENEWIDGET_H_INCLUDED__
#define __SCENEWIDGET_H_INCLUDED__

#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>


// forward declarations
class IScene;
class IModel;
class IMeshModel;


//=============================================================================
//	CSceneWidget
//=============================================================================

/** A configuration widget for IScene.
 * This widget is bound to an IScene object and can be used to modify its parameters.
 */
class CSceneWidget : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a CSceneWidget object.
	 * After the widget has been constructed, it must be initialized with init().
	 * @param scene IScene object to bind to.
	 */
	CSceneWidget( IScene* scene );
	virtual ~CSceneWidget( void ); ///< Destruction.

	/** Initializes the widget.
	 * This must be called after the attached IScene object was successfully initialized,
	 * otherwise behavior is undefined.
	 */
	void init( void );

	/** Must be called before destruction if the widget was initialized.
	 * Frees resources and makes the widget ready for destruction.
	 */
	void shutdown( void );

private slots:
	void checkUseProgram( int toggleState );
	void checkWireframe( int toggleState );
	void checkBackFaceCulling( int toggleState );
	void checkShowOrigin( int toggleState );
	void checkShowNormals( int toggleState );
	void checkShowBoundingBox( int toggleState );
	void checkShowTangents( int toggleState );
	void setActiveModel( int index );
	void resetCamera( bool );
	void selectClearColor( bool );
	void loadMesh( bool );
	void setGeometryOutputType( int index );
	void setProjectionMode( int index );
	void setFov( int index );

private:

	// widgets
	QComboBox*		m_activeModel;
	QComboBox*		m_geometryOutputType;
	QComboBox*		m_projectionMode;
	QComboBox*		m_fov;
	QCheckBox*		m_chkUseProgram;
	QCheckBox*		m_chkWireframe;
	QCheckBox*		m_chkBackFaceCulling;
	QCheckBox*		m_chkShowOrigin;
	QCheckBox*		m_chkShowNormals;
	QCheckBox*		m_chkShowBoundingBox;
	QCheckBox*		m_chkShowTangents;
	QPushButton*	m_btnClearColor;
	QPushButton*	m_btnResetCamera;
	QPushButton*	m_btnLoadMesh;
	QLabel*			m_labPrimitiveType;
	QGroupBox*		m_groupGeometryShader;

	// test models are stored here.
	IModel**	m_models; // [ m_numModels ]
	int			m_numModels;
	IMeshModel*	m_meshModel; // this points into m_models !!!!
	int			m_meshModelIndex; // index into m_models
	QString		m_meshFileName;

	// the scene to modify
	IScene*		m_scene;
};

#endif	// __SCENEWIDGET_H_INCLUDED__
