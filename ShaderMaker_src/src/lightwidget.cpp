//=============================================================================
/** @file		lightwidget.cpp
 *
 * Implements CLightWidget and CMaterialWidget.
 *
	@internal
	created:	2007-11-05
	last mod:	2008-01-07

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QGridLayout>
#include <QLabel>
#include <QColorDialog>
#include "lightwidget.h"



//=============================================================================
//	Lighting and Materials Manual Page
//=============================================================================

/** @page lighting Lighting and materials
This sections explains how to use the 'Lighting' and 'Material' tabs of the editor.

Lighting in OpenGL means computing vertex colors based on lighting and material state.
Lighting and Material state is available in GLSL through several built-in uniform variables
(like gl_LightSource and gl_FrontMaterial for example).
With the 'Lighting' and the 'Material' tabs you can manipulate a subset of the OpenGL
lighting and material state. These include light positions, colors, shininess and some
additional flags.

To learn the purpose of every parameter, you can use the fixed function pipeline.
Go to the 'Scene' tab and uncheck the 'Use GLSL progam' check-box.
Then go to the 'Lighting' tab and check the 'Enable Lighting' check box.
Note that this check-box only affects the fixed function pipeline.
When you use the lighting related uniforms in you shaders, this flag is ignored.
Now you should see the 'Plane' test model not in white but in gray, with a brighter
spot in it's center.

You can see light, but you can't see where it comes from.
To see that, you can check the 'Show lights' check-box.
Now you should see a white sphere in the center of the plane.
This sphere is drawn in the light's diffuse color (see below).

Now move around the light by manipulationg the values of GL_POSITION.
As you might notice, the W component of GL_POSITION changes the lighting effect,
but does not change the light position.
This is caused by the way how light positions are handled in OpenGL ( see the specification for details ).
The shader editor only takes the XYZ components and draws the light source at that location.
But in GLSL you have access to the unchanged values you entered in the GL_POSITON fields.
You should try the other test models as well, but be sure that the light source is outside the model,
or you won't see anything!

The 'Lock To Camera' check-box controls the origin of the light source.
If this box is unchecked, the light position is specified in world-space coordinates.
That means, a position of (0,0,0) is in the center of the test model. If you rotate the
test model, the light rotates with it, because it's 'locked' to the world.
If you check this box, the light position is specified in eye-space coordinates.
Eye space means, that the eye ( or the camera ) is located at (0,0,0) and looks along -Z.
Light sources specified in eye-space coordinates are not transformed if you rotate the test model,
because they are 'locked' to the camera.
This allows for kind of a head-light, for instance.
It is IMPORTANT to note that light sources specified in world-space are transformed by the
model-view matrix and if you read the light's position is GLSL via gl_LightSource[i].position you
will get the TRANSFORMED position! So you won't have to transform them 'by hand'.

If you do not want to change the light position by hand, you can check the 'Auto-rotate' check-box.
This rotates the light source around the light source's origin.


The next step is to modify the light source's colors.
Light sources emit ambient light, diffuse light and specular light.
These light colors are multiplied with the material's light colors.
You can set the colors of the light source in the 'Lighting' tab and
the colors of the material in the 'Materials' tab.
GL_AMBIENT colors are simple multiplied,
GL_DIFFUSE colors are extra multiplied with the cosine of the angle between the
light position and the vertex normal
and GL_SPECULAR colors are multiplies with o coefficient depending on the view-angles
( see the OpenGL specification for details ).
The easies way to find out how lighting works is to play around with it.
Now you have the chance!


OpenGL supports several light sources. You can't edit two light sources at the same time,
you have to select a light source to edit. The light sources are named GL_LIGHT0 to GL_LIGHT7.
You can enable and disable light sources with the 'Enable' check-box.
If a light source is not enabled, it won't be used in OpenGL's lighting calculations.

There are two additional notes for materials:
At first, materials know a fourth color, GL_EMISSION. This color is added to the result of
the lighting equation and represents a self-illumination color.
Second, there is a check-box 'Use vertex color as GL_DIFFUSE'. This check box
allows you to override the value of the GL_DIFFUSE material parameter with the color
value in the model's vertex color.
You can see the behavior with the cube or the sphere test model.

The next tutorial deals with @ref uniforms.
*/


//=============================================================================
//	CLightParamWidget implementation
//=============================================================================

// construction
CLightParamWidget::CLightParamWidget( const QString & name, bool isColorWidget )
 : QGroupBox( name )
{
	m_skipValueChangedSignal = 0;

	QGridLayout* layout = new QGridLayout();

	// create spin boxes
	for( int i = 0 ; i < 4 ; i++ )
	{
		m_vector[i] = new QDoubleSpinBox();

		// clamp to [0,1]
		if( isColorWidget )
		{
			m_vector[i]->setRange( 0.0, 1.0 );
			m_vector[i]->setSingleStep( 0.05f );
		}
		else // unlimited range
		{
			m_vector[i]->setRange( -99.99, +99.99 );
			m_vector[i]->setSingleStep( 0.1f );
		}

		layout->addWidget( m_vector[i], 0, i );
		connect( m_vector[i], SIGNAL(valueChanged(double)), this, SLOT(someValueChanged(double)) );
	}

	//
	// fake button - helper to align the widgets...
	//
	class CFakeButton : public QPushButton
	{
		bool m_fake;
	public:
		CFakeButton( const QString & text, bool fake ) : QPushButton( text ), m_fake( fake ) {}
		void paintEvent( QPaintEvent* e ) { if( !m_fake ) QPushButton::paintEvent(e); }
	};

	//
	// always add the button, disable if not needed
	// -> looks quite bad...
	m_selectColorButton = new CFakeButton( tr( "" ), !isColorWidget );
	layout->addWidget( m_selectColorButton, 0, 4 );
	if( isColorWidget )
	{
		connect( m_selectColorButton, SIGNAL(clicked(bool)), this, SLOT(selectColor(bool)) );
	}

	setLayout( layout );
}


/*
========================
getValues
========================
*/
vec4_t CLightParamWidget::getValues( void ) const
{
	return vec4_t( 
		(float)m_vector[ 0 ]->value(),
		(float)m_vector[ 1 ]->value(),
		(float)m_vector[ 2 ]->value(),
		(float)m_vector[ 3 ]->value() );
}


/*
========================
setValues
========================
*/
void CLightParamWidget::setValues( const vec4_t & values )
{
	m_vector[ 0 ]->setValue( values.x );
	m_vector[ 1 ]->setValue( values.y );
	m_vector[ 2 ]->setValue( values.z );
	m_vector[ 3 ]->setValue( values.w );
}


/*
========================
someValueChanged
========================
*/
void CLightParamWidget::someValueChanged( double )
{
	// avoid recursions
	if( m_skipValueChangedSignal )
		return;

	m_skipValueChangedSignal++;

	// transform color
	vec4_t v = getValues(); v.colorNormalize(); v = v * 255.0f;
	QColor color( (int)v.x, (int)v.y, (int)v.z );

	// change button color
	QPalette pal = m_selectColorButton->palette();
	pal.setColor( QPalette::Button, color );
	m_selectColorButton->setPalette( pal );

	m_skipValueChangedSignal--;

	// notify people interested in me...
	emit contentsChanged();
}


/*
========================
selectColor

 bring up a color select dialog.
========================
*/
void CLightParamWidget::selectColor( bool )
{
	QColor color = m_selectColorButton->palette().color( QPalette::Button );
	color = QColorDialog::getColor( color, this );

	if( color.isValid() )
	{
		vec4_t v( color.red(), color.green(), color.blue(), color.alpha() );
		setValues( v * (1.0f/255.0f) );
	}
}


//=============================================================================
//	CMaterialWidget
//=============================================================================

CMaterialWidget::CMaterialWidget( ILightingState* ls )
{
	m_lightingState = ls;

	// create widgets
	m_emission = new CLightParamWidget( "GL_EMISSION  (r,g,b,a)" );
	m_ambient  = new CLightParamWidget( "GL_AMBIENT  (r,g,b,a)" );
	m_diffuse  = new CLightParamWidget( "GL_DIFFUSE  (r,g,b,a)" );
	m_specular = new CLightParamWidget( "GL_SPECULAR  (r,g,b,a)" );
	m_specularExponent = new QDoubleSpinBox();
	m_useColorMaterial = new QCheckBox( "Use Vertex Color as GL_DIFFUSE" );
	QLabel* specExpLabel = new QLabel( "GL_SHININESS" );

	// GL spec...
	m_specularExponent->setRange( 0.0, 128.0 );

	// setup layout
	QGridLayout* layout = new QGridLayout();
	layout->addWidget( m_emission,			0,0, 1,2 );
	layout->addWidget( m_ambient,			1,0, 1,2 );
	layout->addWidget( m_diffuse,			2,0, 1,2 );
	layout->addWidget( m_specular,			3,0, 1,2 );
	layout->addWidget( specExpLabel,		4,0, 1,1 );
	layout->addWidget( m_specularExponent,	4,1, 1,1 );
	layout->addWidget( m_useColorMaterial,  5,0, 1,2 );
	setLayout( layout );

	// setup connections
	connect( m_emission, SIGNAL(contentsChanged()), this, SLOT(materialPropertyChanged()) );
	connect( m_ambient,  SIGNAL(contentsChanged()), this, SLOT(materialPropertyChanged()) );
	connect( m_diffuse,  SIGNAL(contentsChanged()), this, SLOT(materialPropertyChanged()) );
	connect( m_specular, SIGNAL(contentsChanged()), this, SLOT(materialPropertyChanged()) );
	connect( m_specularExponent, SIGNAL(valueChanged(double)), this, SLOT(materialPropertyChanged(double)) );
	connect( m_useColorMaterial, SIGNAL(stateChanged(int)), this, SLOT(materialPropertyChanged(int)) );

	// upload initial material state
	uploadMaterialState();
}


/*
========================
materialPropertyChanged
========================
*/
void CMaterialWidget::materialPropertyChanged( int )
{
	materialPropertyChanged();
}
void CMaterialWidget::materialPropertyChanged( double )
{
	materialPropertyChanged();
}
void CMaterialWidget::materialPropertyChanged( void )
{
	downloadMaterialState();
}


/*
========================
downloadMaterialState
========================
*/
void CMaterialWidget::downloadMaterialState( void )
{
	CMaterial m;

	m.setEmission( m_emission->getValues() );
	m.setAmbient ( m_ambient->getValues() );
	m.setDiffuse ( m_diffuse->getValues() );
	m.setSpecular( m_specular->getValues() );
	m.setSpecularExponent( (float)m_specularExponent->value() );
	m.setUseVertexColor( m_useColorMaterial->checkState() != Qt::Unchecked );

	m_lightingState->setMaterial( m );
}


/*
========================
uploadMaterial
========================
*/
void CMaterialWidget::uploadMaterialState( void )
{
	CMaterial m;
	m_lightingState->getMaterial( m );

	m_emission->setValues( m.getEmission() );
	m_ambient->setValues ( m.getAmbient() );
	m_diffuse->setValues ( m.getDiffuse() );
	m_specular->setValues( m.getSpecular() );
	m_specularExponent->setValue( m.getSpecularExponent() );
	m_useColorMaterial->setCheckState( m.getUseVertexColor() ? Qt::Checked : Qt::Unchecked );
}


//=============================================================================
//	CLightWidget implementation
//=============================================================================

// construction
CLightWidget::CLightWidget( ILightingState* ls )
 : m_lightingState( ls )
{
	m_skipSetActiveLight = 1;
	m_skipLightPropertiesChanged = 1;

	QVBoxLayout* layout = new QVBoxLayout( this );

	//
	// header controls
	//
	m_enableLighting	= new QCheckBox( tr( "Enable Lighting" ) );
	m_showLights		= new QCheckBox( tr( "Show Lights" ) );
	m_enableLight		= new QCheckBox( tr( "Enabled" ) );
	m_lockToCamera		= new QCheckBox( tr( "Lock To Camera" ) );
	m_autoRotateLight	= new QCheckBox( tr( "Auto-Rotate" ) );
	m_activeLight		= new QComboBox();
	for( int i = 0 ; i < ILightingState::MAX_LIGHTS ; i++ )
	{
		m_activeLight->insertItem( i, tr( "GL_LIGHT%1" ).arg(i) );
	}
	QGridLayout* header = new QGridLayout();
	header->addWidget( m_enableLighting, 0,0, 1,2 );
	header->addWidget( m_showLights,     0,2, 1,2 );
	header->addWidget( new QLabel( tr( "Lightsource:" ) ), 1, 0 );
	header->addWidget( m_activeLight,     1, 1 );
	header->addWidget( m_enableLight,     1, 2 );
	header->addWidget( m_lockToCamera,    1, 3 );
	header->addWidget( m_autoRotateLight, 1, 4 );
	header->setAlignment( m_enableLight, Qt::AlignRight );
	layout->addLayout( header );

	//
	// parameter widgets
	//
	layout->addWidget( m_position = new CLightParamWidget( "GL_POSITION  (x,y,z,w)", false ) );
	layout->addWidget( m_ambient  = new CLightParamWidget( "GL_AMBIENT  (r,g,b,a)" ) );
	layout->addWidget( m_diffuse  = new CLightParamWidget( "GL_DIFFUSE  (r,g,b,a)" ) );
	layout->addWidget( m_specular = new CLightParamWidget( "GL_SPECULAR  (r,g,b,a)" ) );

	setLayout( layout );

	// setup connections
	connect( m_activeLight,		SIGNAL(currentIndexChanged(int)), this, SLOT(setActiveLight(int)) );
	connect( m_position,		SIGNAL(contentsChanged()), this, SLOT(lightPropertiesChanged()) );
	connect( m_ambient,			SIGNAL(contentsChanged()), this, SLOT(lightPropertiesChanged()) );
	connect( m_diffuse,			SIGNAL(contentsChanged()), this, SLOT(lightPropertiesChanged()) );
	connect( m_specular,		SIGNAL(contentsChanged()), this, SLOT(lightPropertiesChanged()) );
	connect( m_enableLight,		SIGNAL(stateChanged(int)), this, SLOT(lightPropertiesChanged(int)) );
	connect( m_autoRotateLight, SIGNAL(stateChanged(int)), this, SLOT(lightPropertiesChanged(int)) );
	connect( m_lockToCamera,	SIGNAL(stateChanged(int)), this, SLOT(lightPropertiesChanged(int)) );
	connect( m_enableLighting,	SIGNAL(stateChanged(int)), this, SLOT(lightingEnabledChanged(int)) );
	connect( m_showLights,      SIGNAL(stateChanged(int)), this, SLOT(showLightsChanged(int)) );

	// enable signal processing
	m_skipSetActiveLight = 0;
	m_skipLightPropertiesChanged = 0;

	// trigger an upload event
	setActiveLight( 0 );
}


/*
========================
setActiveLight
========================
*/
void CLightWidget::setActiveLight( int lightIndex )
{
	if( m_skipSetActiveLight )
		return;

	// out of range... should never happen
	if( lightIndex < 0 || lightIndex >= ILightingState::MAX_LIGHTS )
		return;

	m_skipSetActiveLight++;
	m_skipLightPropertiesChanged++;

	CLight l;

	// set new content
	// -> no need to download current values, this is done
	//    every time something changes.
	m_activeLight->setCurrentIndex( lightIndex );
	m_lightingState->getLight( lightIndex, l );
	uploadLight( l );

	m_skipLightPropertiesChanged--;
	m_skipSetActiveLight--;
}


/*
========================
lightPropertiesChanged

 signalled when one of the light param widgets changes state
========================
*/
void CLightWidget::lightPropertiesChanged( int )
{
	lightPropertiesChanged();
}
void CLightWidget::lightPropertiesChanged( void )
{
	if( m_skipLightPropertiesChanged )
		return;

	m_skipLightPropertiesChanged++;

	// save changes
	int index = m_activeLight->currentIndex();
	if( index != -1 )
	{
		CLight l;
		downloadLight( l );
		m_lightingState->setLight( index, l );
	}

	m_skipLightPropertiesChanged--;
}


/*
========================
lightingEnabledChanged
========================
*/
void CLightWidget::lightingEnabledChanged( int state )
{
	m_lightingState->setLightingEnabled( state != Qt::Unchecked );
}


/*
========================
showLightsChanged
========================
*/
void CLightWidget::showLightsChanged( int state )
{
	m_lightingState->setShowLights( state != Qt::Unchecked );
}


/*
========================
downloadLight

 writes current widget state into buffer
========================
*/
void CLightWidget::downloadLight( CLight & l )
{
	l.setEnabled		( m_enableLight->checkState() != Qt::Unchecked );
	l.setLockedToCamera	( m_lockToCamera->checkState() != Qt::Unchecked );
	l.setAutoRotate		( m_autoRotateLight->checkState() != Qt::Unchecked );
	l.setPosition		( m_position->getValues() );
	l.setAmbient		( m_ambient->getValues()  );
	l.setDiffuse		( m_diffuse->getValues()  );
	l.setSpecular		( m_specular->getValues() );
}


/*
========================
uploadLight

 reads buffer and sets new widget state
========================
*/
void CLightWidget::uploadLight( const CLight & l )
{
	m_enableLight->setCheckState( l.getEnabled() ? Qt::Checked : Qt::Unchecked );
	m_lockToCamera->setCheckState( l.getLockedToCamera() ? Qt::Checked : Qt::Unchecked );
	m_autoRotateLight->setCheckState( l.getAutoRotate() ? Qt::Checked : Qt::Unchecked );
	m_position->setValues( l.getPosition() );
	m_ambient ->setValues( l.getAmbient()  );
	m_diffuse ->setValues( l.getDiffuse()  );
	m_specular->setValues( l.getSpecular() );
}






