//=============================================================================
/** @file		uniformwidget.cpp
 *
 * Implements CUniformWidget.
 *
	@internal
	created:	2007-11-17
	last mod:	2008-02-16

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QtCore/QSignalMapper>
#include <QGridLayout>
#include <QColorDialog>
#include <QApplication>
#include "application.h"
#include "uniformwidget.h"
#include "universalslider.h"
#include "vector.h"
#include "uniform.h"


//=============================================================================
//	Uniforms man page
//=============================================================================

/** @page uniforms Uniform variables
This tutorial explains the use of uniforms variables.

Uniform variable are a kind of 'global' variables that a shader can read.
An application communicates with the shader by using uniform variables.
In the shader editor, you can set the values of the uniforms you define
with the 'Uniforms' tab.
As an example how to use it, mofify the initial fragment shader in the following way:
Add the line

uniform vec4 colorFromApp;

to the shader at global scope.
This line defines a global variable (visible in the fragment shader) that can
be set by the application.
You can read this variable like any other variable, like for example:

gl_FragColor = colorFromApp;

Now you see nothing. The default value any uniforms is zero.
And in this case it is a zero vector used as a color, which OpenGL interprets as black.
Got to the 'Uniforms' tab and move around the XYZ sliders. Now you sould see the test model
in a color you selected by setting the XYZ components to a RGB value.
You can also select a color by using the colored button in the widget.
Color selection works only with 3D and 4D float vectors.

If you define several uniforms, you can select uniform you want to manipulate in a combo box.
Right to that combo box you can see the data type of that uniform.
Data types define the values a uniform variable can store.
For example, GL_BOOL only knows true and false, where GL_INT knows signed integer values.

You can use the sliders to select a value or you can enter the value directly in the
spin box on the lest side. If you want to change the range of the slider, you can use the spin
boxes on the left and the right side of the slider.
If you want to edit matrix types, like GL_FLOAT_MAT3, you can only edit one matrix column at the same time.
You can change the matrix column with the 'Active Matrix Column' spin box on the bottom of the widget.
Matrices are initialized to identity matrices and not to zero matrices.

Uniform variables are available at all shader types, but they must be defined in all shaders that use them.
If you define a uniform with the equal name but different type, the results are undefined!

There is one special uniform you cannot edit with the 'Uniforms' tab.
This special uniform is declared as

uniform float time;

If you define a float uniform variable named 'time', the shader editor interprets it
as a variable, that represents a counter of seconds since the GLSL program was linked.
Because of that, the value of 'time' is set internally by the shader editor.
You can test this functionality with the following code:

gl_FragColor = gl_Color * ( 0.5 + 0.5 * sin( time ) );


Now go on to the last tutorial, @ref geometryshader.
*/


//=============================================================================
//	CUniformWidget implementation
//=============================================================================

// construction
CUniformWidget::CUniformWidget( IUniformState* us )
{
	m_uniformState = us;

	// disable signal processing at startup
	m_skipComponentChanged = 1;
	m_skipSetActiveUniform = 1;
	m_skipSetActiveMatrixColumn = 1;

	m_lastActiveUniform = -1; // none

	m_numUniforms = 0;
	m_indexMap = NULL;

	QGridLayout* layout = new QGridLayout();
	QSignalMapper* mapper = new QSignalMapper( this );

	m_activeUniform = new QComboBox();
	layout->addWidget( m_activeUniform, 0,0 );

	m_typeLabel = new QLabel( "" );
	layout->addWidget( m_typeLabel, 0, 1 );

	for( int i = 0 ; i < 4 ; i++ )
	{
		QString name;
		switch( i )
		{
		case 0: name = QString( "X" ); break;
		case 1: name = QString( "Y" ); break;
		case 2: name = QString( "Z" ); break;
		case 3: name = QString( "W" ); break;
		}

		m_components[ i ] = new CUniversalSlider( name );
		connect( m_components[ i ], SIGNAL(contentsChanged()), mapper, SLOT(map()) );
		mapper->setMapping( m_components[ i ], i );
		layout->addWidget( m_components[ i ], i+1, 0, 1, 2 );
	}

	m_colorButton = new QPushButton( "" );
	layout->addWidget( new QLabel( tr( "Select color:" ) ), 5, 0 );
	layout->addWidget( m_colorButton, 5, 1 );

	m_activeMatrixColumn = new QSpinBox();
	layout->addWidget( new QLabel( tr( "Active Matrix Column:" ) ), 6, 0 );
	layout->addWidget( m_activeMatrixColumn, 6, 1 );

	setLayout( layout );

	connect( mapper, SIGNAL(mapped(int)), this, SLOT(componentChanged(int)) );
	connect( m_activeUniform, SIGNAL(currentIndexChanged(int)), this, SLOT(setActiveUniform(int)) );
	connect( m_activeMatrixColumn, SIGNAL(valueChanged(int)), this, SLOT(setActiveMatrixColumn(int)) );
	connect( m_colorButton, SIGNAL(clicked(bool)), this, SLOT(selectColor(bool)) );

	// enable signal processing
	m_skipComponentChanged = 0;
	m_skipSetActiveUniform = 0;
	m_skipSetActiveMatrixColumn = 0;
}

CUniformWidget::~CUniformWidget( void )
{
	SAFE_DELETE_ARRAY( m_indexMap );
}


/*
========================
updateUniformList
========================
*/
void CUniformWidget::updateUniformList( void )
{
	clearContent();
	readContentFromState();
}


/*
========================
setActiveUniform
========================
*/
void CUniformWidget::setActiveUniform( int active )
{
	if( m_skipSetActiveUniform )
		return;

	CUniform u;

	// do not interfere...
	m_skipComponentChanged++;
	m_skipSetActiveUniform++;

	// save old content
	if( m_lastActiveUniform != -1 )
	{
		u = m_uniformState->getUniform( m_indexMap[ m_lastActiveUniform ] );
		downloadUniform( u, -1 );
		m_uniformState->setUniform( m_indexMap[ m_lastActiveUniform ], u );
	}

	// upload new content
	u = m_uniformState->getUniform( m_indexMap[ active ] );
	uploadUniform( u );
	m_lastActiveUniform = active;

	m_skipSetActiveUniform--;
	m_skipComponentChanged--;
}


/*
========================
componentChanged
========================
*/
void CUniformWidget::componentChanged( int component )
{
	if( m_skipComponentChanged )
		return;

	// prevent recursions...
	m_skipComponentChanged++;

	// get type info etc.
	CUniform u;
	loadActiveUniform( u );

	// download content into buffer
	downloadUniform( u, component );
	updateColorButton( u );

	// give it back to the state manager
	saveActiveUniform( u );

	m_skipComponentChanged--;
}


/*
========================
readContentFromState
========================
*/
void CUniformWidget::readContentFromState( void )
{
	int i,j;

	// do not process each change signal...
	m_skipComponentChanged++;
	m_skipSetActiveUniform++; // ignore while filling the combo box

	// alloc index map
	m_numUniforms = m_uniformState->getActiveUniforms();
	m_indexMap = new int [ m_numUniforms ];

	// process each uniform
	for( i = j = 0 ; i < m_numUniforms ; i++ )
	{
		// check wether this type can be handled
		CUniform u = m_uniformState->getUniform( i );
		if( !acceptsUniform( u ) ) // not supported
		{
			continue;
		}

		// add it
		m_activeUniform->addItem( u.getName() );
		m_indexMap[ j++ ] = i;
	}

	// we are going to upload data via setActiveUniform
	m_skipSetActiveUniform--;

	// if there are any uniforms to edit
	if( j > 0 )
	{
		setEnabled( true );
		setActiveUniform( 0 );
	}
	else
	{
		setEnabled( false );
	}

	m_skipComponentChanged--;
}


/*
========================
clearContent
========================
*/
void CUniformWidget::clearContent( void )
{
	// do not read back any values!
	m_skipSetActiveUniform++;

	m_activeUniform->clear();

	SAFE_DELETE_ARRAY( m_indexMap );
	m_numUniforms = 0;

	// clear color button
	updateColorButton( CUniform() );

	m_lastActiveUniform = -1; // none

	m_skipSetActiveUniform--;
}


/*
========================
uploadUniform

 Sets widget content to the uniform's data.
 Also update matrix column, color and type info controls.
 Assumes the uniform is validated by accepsUniform().
========================
*/
void CUniformWidget::uploadUniform( const CUniform & u )
{
	// upload type info label
	m_typeLabel->setText( u.getTypeName() );

	uploadUniformData( u );

	// update helper controls
	updateColorButton( u );
	updateMatrixColumnBox( u );
}


/*
========================
uploadUniformData

 sets widget content to the uniform's data.
 assumes the uniform is validated by accepsUniform().
========================
*/
void CUniformWidget::uploadUniformData( const CUniform & u )
{
	int baseType = u.getBaseType();

	// upload components
	for( int i = 0 ; i < 4 ; i++ )
	{
		bool enable = true;

		// map to base type
		switch( baseType )
		{
		case CUniform::BASE_TYPE_BOOL:
		{
			m_components[ i ]->setContentBool( u.getValueAsBool( i ) );
			break;
		}

		case CUniform::BASE_TYPE_INT:
		{
			int value = u.getValueAsInt( i );
			m_components[ i ]->setContentInt( qMin(0,value), qMax(100,value), value );
			break;
		}

		// this should handle matrices too, it uploads column 0.
		case CUniform::BASE_TYPE_FLOAT:
		{
			double value = u.getValueAsFloat( i );
			m_components[ i ]->setContentFloat( qMin(0.0,value), qMax(1.0,value), value );
			break;
		}

		// not used
		default:
			enable = false;
			break;
		}

		// disable unused components
		if( i >= u.getComponentCount() )
			enable = false;

		// disable if not used
		m_components[ i ]->setEnabled( enable );
	}
}


/*
========================
downloadUniform

 Stores widget content in u.
 If component is 0,1,2 or 3 only that component will be
 downloaded, otherwise all components will be downloaded.
========================
*/
void CUniformWidget::downloadUniform( CUniform & u, int component )
{
	// handle matrices
	if( u.isMatrix() )
	{
		int activeColumn = m_activeMatrixColumn->value();
		CUniform column = u.getColumnVector( activeColumn );
		downloadUniform( column, component );
		u.setColumnVector( activeColumn, column );
		return;
	}

	// precache mapped values
	int i = 0;
	int components = u.getComponentCount();
	int baseType = u.getBaseType();

	// limit the loop to one component
	if( component >= 0 && component < 4 )
	{
		i = component;
		components = i+1;
	}

	// loop through the selected components
	for( ; i < components ; i++ )
	{
		// do type dependent actions...
		switch( baseType )
		{
		case CUniform::BASE_TYPE_BOOL:
			u.setValueAsBool( i, m_components[ i ]->getContentBool() );
			break;

		case CUniform::BASE_TYPE_INT:
			u.setValueAsInt( i, m_components[ i ]->getContentInt() );
			break;

		case CUniform::BASE_TYPE_FLOAT:
			u.setValueAsFloat( i, m_components[ i ]->getContentFloat() );
			break;
		}
	}
}


/*
========================
setActiveMatrixColumn

 assumes this is only called on matrices!
========================
*/
void CUniformWidget::setActiveMatrixColumn( int column )
{
	if( m_skipSetActiveMatrixColumn )
		return;

	// widget changes, not the componet's value!
	m_skipComponentChanged++;
	m_skipSetActiveMatrixColumn++;

	CUniform u;
	loadActiveUniform( u );
	uploadUniformData( u.getColumnVector( column ) );

	m_skipComponentChanged--;
	m_skipSetActiveMatrixColumn--;
}


/*
========================
updateMatrixColumnBox

 Use to initialize the current uniform.
 Does NOT emit active column signals.
========================
*/
void CUniformWidget::updateMatrixColumnBox( const CUniform & u )
{
	m_skipSetActiveMatrixColumn++;

	// enable spin box
	if( u.isMatrix() )
	{
		m_activeMatrixColumn->setRange( 0, u.getColumnCount() - 1 );
		m_activeMatrixColumn->setValue( 0 );
		m_activeMatrixColumn->setEnabled( true );
	}

	// disable spin box
	else
	{
		m_activeMatrixColumn->setRange( 0,0 );
		m_activeMatrixColumn->setValue( 0 );
		m_activeMatrixColumn->setEnabled( false );
	}

	m_skipSetActiveMatrixColumn--;
}


/*
========================
updateColorButton

 sets the color button for the active uniform
========================
*/
void CUniformWidget::updateColorButton( const CUniform & u )
{
	QPalette pal = m_colorButton->palette();

	// a possible color value
	if( !u.isMatrix() && // assume matrices do not store colors
		u.getBaseType() == u.BASE_TYPE_FLOAT &&
		u.getComponentCount() >= 3 )
	{
		// read color value
		vec4_t v(	u.getValueAsFloat( 0 ),
					u.getValueAsFloat( 1 ),
					u.getValueAsFloat( 2 ),
					u.getValueAsFloat( 3 ) );
		v.colorNormalize();
		v = v * 255.0f;
		QColor color( (int)v.x, (int)v.y, (int)v.z );

		pal.setColor( QPalette::Button, color );
		m_colorButton->setEnabled( true );
	}

	// not a color value
	else
	{
		pal.setColor( QPalette::Button, qApp->palette().color( QPalette::Button ) );
		m_colorButton->setEnabled( false );
	}

	m_colorButton->setPalette( pal );
}


/*
========================
selectColor

 assumes this is only called on 'color' uniforms...
========================
*/
void CUniformWidget::selectColor( bool )
{
	// ask for the new color
	QColor color = m_colorButton->palette().color( QPalette::Button );
	color = QColorDialog::getColor( color, this );

	if( color.isValid() )
	{
		// get uniform
		CUniform u;
		loadActiveUniform( u );

		// update state
		u.setValueAsFloat( 0, color.red()   / 255.0 );
		u.setValueAsFloat( 1, color.green() / 255.0 );
		u.setValueAsFloat( 2, color.blue()  / 255.0 );

		// update widgets
		m_skipComponentChanged++;
		uploadUniform( u );
		m_skipComponentChanged--;

		saveActiveUniform( u );
	}
}


/*
========================
loadActiveUniform

 use this instead of directly mapping indices each time...
========================
*/
void CUniformWidget::loadActiveUniform( CUniform & u )
{
	int active = m_activeUniform->currentIndex();
	u = m_uniformState->getUniform( m_indexMap[ active ] );
}


/*
========================
saveActiveUniform
========================
*/
void CUniformWidget::saveActiveUniform( const CUniform & u )
{
	int active = m_activeUniform->currentIndex();
	m_uniformState->setUniform( m_indexMap[ active ], u );
}


/*
========================
accpetsUniform

 used to filter out unsupported types.
 only 'simple' types can be edited
 -> no samplers
========================
*/
bool CUniformWidget::acceptsUniform( const CUniform & u )
{
	// not editable
	if( u.getLocation() == -1 )
		return false;

	// Evil Hack:
	//  time is constantly increasing, so it is not useful to
	//  make it editable by hand...
	if( u.getType() == GL_FLOAT &&
		0 == QString("time").compare( u.getName(), Qt::CaseInsensitive )  )
	{
		return false;
	}

	// matrices are edited by editing their column vectors
	if( u.isMatrix() && u.getBaseType() == CUniform::BASE_TYPE_FLOAT )
		return true;

	// type based filtering
	switch( u.getBaseType() )
	{
	case CUniform::BASE_TYPE_BOOL:
	case CUniform::BASE_TYPE_INT:
	case CUniform::BASE_TYPE_FLOAT:
		return true;
		break;

	default:
		return false;
		break;
	}

	return false;
}


