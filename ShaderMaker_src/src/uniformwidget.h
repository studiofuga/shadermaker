//=============================================================================
/** @file		uniformwidget.h
 *
 * Defines the uniform variable editing widget
 *
	@internal
	created:	2007-11-16
	last mod:	2007-12-16

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __UNIFORMWIDGET_H_INCLUDED__
#define __UNIFORMWIDGET_H_INCLUDED__

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>


// forward declarations
class CUniform;
class CUniversalSlider;
class IUniformState;


//=============================================================================
//	CUniformWidget
//=============================================================================


/** Widget for manipulating IUniformState.
 *  This is the user interface for the IUniformState interface.
 *  It show the active uniforms in the IUniformState object and
 *  allows manipulation if those values.
 *  This widgets supports vectors of type boolean, integer and float
 *  with up to four components.
 *  In addition it supports float matrix maninipulation for up to 4x4 matrices.
 */
class CUniformWidget : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a CUniformWidget bound to a IUniformState object.
	 * @param us IUniformState object to bind to.
	 * @pre The IUniformState object must live longer than the CUniformWidget,
	 *      otherwise the pointer will be corrupted.
	 */
	CUniformWidget( IUniformState* us );
	virtual ~CUniformWidget( void ); ///< Destructor.

public slots:
	/** Forces the widget to update its content with
	 * the values and uniforms currently stored
	 * in the bound IUniformState object.
	 */
	void updateUniformList( void );

private slots:
	void componentChanged( int component );
	void setActiveUniform( int active );
	void setActiveMatrixColumn( int column );
	void selectColor( bool );

private:

	void clearContent( void );
	void readContentFromState( void );

	// accesses the active uniform, selected by m_activeUniform
	void loadActiveUniform( CUniform & u );
	void saveActiveUniform( const CUniform & u );

	void updateColorButton( const CUniform & u );
	void updateMatrixColumnBox( const CUniform & u );

	void downloadUniform( CUniform & u, int component );
	void uploadUniform( const CUniform & u );
	void uploadUniformData( const CUniform & u );

	bool acceptsUniform( const CUniform & u );

	// maps combo box indices to uniform state indices
	int* m_indexMap; // [ m_numUniforms ]
	int  m_numUniforms;

	// skip signals if != 0
	int m_skipComponentChanged;
	int m_skipSetActiveUniform;
	int m_skipSetActiveMatrixColumn;

	// used to react to setActiveUniform
	int m_lastActiveUniform;

	// widgets
	QComboBox*			m_activeUniform;
	QSpinBox*			m_activeMatrixColumn;
	QPushButton*		m_colorButton;
	QLabel*				m_typeLabel; // type info of the active uniform
	CUniversalSlider*	m_components[ 4 ];

	// where the infos are applied to
	IUniformState*	m_uniformState;
};


#endif	// __UNIFORMWIDGET_H_INCLUDED__

