//=============================================================================
/** @file		lightwidget.h
 *
 * Defines the lighting and material state configuration widgets.
 *
	@internal
	created:	2007-11-05
	last mod:	2007-12-17

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __LIGHTWIDGET_H_INCLUDED__
#define __LIGHTWIDGET_H_INCLUDED__

#include <QWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>

#include "light.h"


//=============================================================================
//	CLightParamWidget
//=============================================================================

/** 4D color manipulation widget.
 * This widget is a collection of four spin boxes to select and RGBA value
 * and a push button that brings up a color selection dialog.
 * The intention is, that the user can select colors when seeing them and
 * then manipulate them without bringing up the color dialog every time.
 * The widget can also be used to ask the user for a 4D position vector.
 * In this case, the color selection button is invisible, but still takes space
 * of the widget. This should make it easy to align it with other CLightParam
 * widgets.
 */
class CLightParamWidget : public QGroupBox
{
	Q_OBJECT
public:
	/** Constructs a light oaram widget.
	 * @param name Name of the property to edit. This name is displayed to the user.
	 * @param isColorWidget Flag wether this widget is used as a color input 
	 *			widget (the default). If this is set to False, the  the color
	 *			selection button will not be available.
	 */
	CLightParamWidget( const QString & name=QString(), bool isColorWidget=true );

	/** Returns the current values stored in this widget.
	 */
	vec4_t getValues( void ) const;

	/** Sets the values stored in this widget.
	 * @param values New values.
	 */
	void setValues( const vec4_t & values );

signals:
	/** Emitted when the user changes data in the widget. */
	void contentsChanged( void );

private slots:
	void someValueChanged( double );
	void selectColor( bool );

private:
	int				m_skipValueChangedSignal;
	QDoubleSpinBox*	m_vector[ 4 ];
	QPushButton*	m_selectColorButton;
};


//=============================================================================
//	CLightWidget
//=============================================================================

/** Widget for manipulating ILightingState.
 * This withget is bound to a ILightingState object and then displays
 * and manipulates its content. The ILightingState object must live longer
 * than the CLightWidget object.
 * \n\n
 * This widget supports the manipulation of all state defined
 * in the ILightingState interface, except the current material state.
 * That state can be manipulated with a CMaterialWidget object.
 */
class CLightWidget : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a lighting widget.
	 * @param ls ILightState object to manipulate.
	 */
	CLightWidget( ILightingState* ls );


private slots:
	void setActiveLight( int lightIndex );
	void lightPropertiesChanged( void );
	void lightPropertiesChanged( int );
	void lightingEnabledChanged( int );
	void showLightsChanged( int );

private:

	// state changing
	void downloadLight( CLight & light );
	void uploadLight( const CLight & light );

	// signal processiong control
	int m_skipSetActiveLight;
	int m_skipLightPropertiesChanged;

	// widgets
	QComboBox*			m_activeLight;
	QCheckBox*			m_enableLight;		// GL_LIGHTi
	QCheckBox*			m_enableLighting;	// GL_LIGHTING
	QCheckBox*			m_lockToCamera;		// for active light
	QCheckBox*			m_autoRotateLight;	// for active light
	QCheckBox*			m_showLights;
	CLightParamWidget*	m_position;
	CLightParamWidget*	m_ambient;
	CLightParamWidget*	m_diffuse;
	CLightParamWidget*	m_specular;

	ILightingState*		m_lightingState;
};


//=============================================================================
//	CMaterialWidget
//=============================================================================

/** Widget for manipulating ILightingState.
 * This withget is bound to a ILightingState object and then displays
 * and manipulates its content. The ILightingState object must live longer
 * than the CLightWidget object.
 * \n\n
 * This widget is dedicated for the material parameters in ILightingState.
 */
class CMaterialWidget : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a material widget.
	 * @param ls ILightingState object to manipulate.
	 */
	CMaterialWidget( ILightingState* ls );

private slots:;
	void materialPropertyChanged( int );
	void materialPropertyChanged( void );
	void materialPropertyChanged( double );

private:

	// helpers
	void uploadMaterialState( void ); // ILightingState -> GUI
	void downloadMaterialState( void ); // GUI -> ILightingState

	// widgets
	CLightParamWidget*	m_emission;
	CLightParamWidget*	m_ambient;
	CLightParamWidget*	m_diffuse;
	CLightParamWidget*	m_specular;
	QDoubleSpinBox*		m_specularExponent;
	QCheckBox*			m_useColorMaterial;

	ILightingState*		m_lightingState;
};


#endif	// __LIGHTWIDGET_H_INCLUDED__
