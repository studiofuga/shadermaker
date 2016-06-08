//=============================================================================
/** @file		texturewidget.cpp
 *
 * Implements CTextureWidget
 *
	@internal
	created:	2007-11-28
	last mod:	2008-02-11

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include "application.h"
#include "texturewidget.h"
#include "texture.h"
#include "uniform.h"


//=============================================================================
//	Textures man page
//=============================================================================

/** @page textures Texturing
This tutorial explains how to use textures and samplers.

Texturing is done in the fragment shader. Some vertex shader implementations are
also capable of accessing textures, but that's not the goal of this tutorial.
First, you need to define a sampler2D uniform variable. This is the access point to texture data.
So add the following line at global scope:

uniform sampler2D myTextureMap;

Now you can read data from this sampler by using the texture2D function.
This functions takes a sampler variable an a texture coordinate vector as arguments.
It is used in the following way:

gl_FragColor = texture2D( myTextureMap, gl_TexCoord[0].st );

This looks up a vec4 value from myTextureMap at the coordinates gl_TexCoord[0].st.
These coordiates are the interpolated result of the vertex shader output. If you do not
write to gl_TexCoord[0] in your vertex shader, the value in the fagment shader is undefined!
The initial vertex shader passes through gl_MultiTexCoord0, which holds the texture coords
of the test model.

If you compile your shader and look at the results, you should see nothing. That's because
you have only defined a sampler, but no texture image! This will be the next step. Go to
the 'Textures' tab and click the button inside the GL_TEXTURE0 box. You can now load an image file
into the texture mapping unit GL_TEXTURE0. Depending on you OpenGL implementation, you have several
texture mapping units available in which you can load texture images. Now you should see the image
mapped onto the test model.

Now click the button in the GL_TEXTURE1 box and load another image.
You will still see the image of GL_TEXTURE0. To use the image in GL_TEXTURE1 you
must set the value of the sampler 'myTextureMap' to 1 in the spin box.

Samplers are basically integers, that identify a texture mapping unit. This way your
shader does not 'see' the details of the OpenGL implementation and you can define multiple
samplers with different semantics but pointing to the same texture mapping unit. It's simply
a level of abstraction between the shader an the application in which the shader is embedded.
You can define more samplers and play around with them to learn the way how texture mapping units
and samplers interact.

The 'Bilinear filtering' check box enables texture filtering. 
It can help to improve visual quality.

Continue with @ref lighting
*/



//=============================================================================
//	CTextureWidget implementation
//=============================================================================

// construction
CTextureWidget::CTextureWidget( ITextureState* ts, IUniformState* us )
{
	m_textureState = ts;
	m_uniformState = us;

	m_mapper = new QSignalMapper( this );

	// enable signal processing
	m_skipSetActiveSampler = 0;
	m_skipSetSamplerTmu = 0;

	// set an initial directory
	m_lastImageDirectory = QString( CONFIG_TEXTURE_DIRECTORY );

	// setup tmus
	m_validTmus = 0;

	m_chkBilinearFilter = new QCheckBox( "Use Bilinear Filtering" );
	connect( m_chkBilinearFilter, SIGNAL(stateChanged(int)), this, SLOT(checkBilinearFilter(int)) );

	// create layout and controls.
	QGridLayout* layout = new QGridLayout();
	QGroupBox* tb = createTmuWidget();
	QGroupBox* sb = createSamplerWidget();
	layout->addWidget( tb, 0,0, 2,1 );
	layout->addWidget( sb, 0,1, 1,1 );
	layout->addWidget( m_chkBilinearFilter, 1,1, 1,1 );
	setLayout( layout );
}


/*
========================
createSamplerWidget
========================
*/
QGroupBox* CTextureWidget::createSamplerWidget( void )
{
	// this box must belong to CTextureWidget,
	// otherwise it would look strange next to the TMU group box.
	QGroupBox* group = new QGroupBox( "Samplers" );
	QGridLayout* groupLayout = new QGridLayout();

	m_activeSampler = new QComboBox();
	m_samplerTmu = new QSpinBox();
	m_activeSampler->setEnabled( false ); // disable until init() is called
	m_samplerTmu->setEnabled( false );
	m_activeSampler->setToolTip( "Select the sampler to edit." );
	m_samplerTmu->setToolTip( "Assigns a Texture Mapping Unit to the selected sampler." );

	groupLayout->addWidget( new QLabel( "Sampler:" ),				0,0, 1,1 );
	groupLayout->addWidget( m_activeSampler,						0,1, 1,2 );
	groupLayout->addWidget( new QLabel( "Texture Mapping Unit:" ),	1,0, 1,2 );
	groupLayout->addWidget( m_samplerTmu,							1,2, 1,1 );

	group->setLayout( groupLayout );

	return group;
}


/*
========================
createTmuWidget

 Creates texture selection buttons and disables them.
 They are enabled in init().
========================
*/
QGroupBox* CTextureWidget::createTmuWidget( void )
{
	QGroupBox* group = new QGroupBox( "Texture Mapping Units" );
	QGridLayout* groupLayout = new QGridLayout();

	for( int i = 0 ; i < MAX_USED_TMUS ; i++ )
	{
		// setup button
		QGridLayout* subLayout = new QGridLayout();
		m_tmus[i].button = new QPushButton( QString( "" ) );
		m_tmus[i].button->setMinimumSize( textureButtonSize() );
		m_tmus[i].button->setMaximumSize( textureButtonSize() );
		subLayout->addWidget( m_tmus[i].button );

		// setup button frame
		m_tmus[i].groupBox = new QGroupBox( QString( "GL_TEXTURE%1" ).arg(i) );
		m_tmus[i].groupBox->setLayout( subLayout );
		m_tmus[i].groupBox->setEnabled( false ); // disable until init().

		groupLayout->addWidget( m_tmus[i].groupBox, i/2, i%2 );
	}

	group->setLayout( groupLayout );
	return group;
}


/*
========================
init

 This will setup the texture controls
 depending on the current rendering context.
 It requires a rendering context, which might not
 be valid during construction.
========================
*/
void CTextureWidget::init( void )
{
	// limit texture units
	m_validTmus = m_textureState->getMaxTextureUnits();
	if( m_validTmus > MAX_USED_TMUS ) // qMin does not work here...
		m_validTmus = MAX_USED_TMUS;

	// enable supported TMUs
	for( int i = 0 ; i < m_validTmus ; i++ )
	{
		m_tmus[i].groupBox->setEnabled( true );

		connect( m_tmus[i].button, SIGNAL(released()), m_mapper, SLOT(map()) );
		m_mapper->setMapping( m_tmus[i].button, i );
	}

	// init sampler widgets
	m_samplerTmu->setRange( 0, m_validTmus-1 );

	// setup connections
	connect( m_activeSampler,	SIGNAL(currentIndexChanged(int)),	this, SLOT(setActiveSampler(int)) );
	connect( m_samplerTmu,		SIGNAL(valueChanged(int)),			this, SLOT(setSamplerTmu(int)) );
	connect( m_mapper,			SIGNAL(mapped(int)),				this, SLOT(selectTexture(int)) );
}


/*
========================
shutdown
========================
*/
void CTextureWidget::shutdown( void )
{
	m_validTmus = 0;
}


/*
========================
textureButtonSize
========================
*/
QSize CTextureWidget::textureButtonSize( void ) const
{
	return QSize( 70, 70 );
}


/*
========================
textureIconSize
========================
*/
QSize CTextureWidget::textureIconSize( void ) const
{
	// should be smaller than textureButtonSize to leave some
	// space for the button border.
	return QSize( 64, 64 );
}


/*
========================
selectTexture

 brings up a 'load image' dialog and loads a texture image into the indexed
 TMU of m_textureState.
========================
*/
void CTextureWidget::selectTexture( int tmu )
{
	// bring up 'load image' dialog.
	QString fileName = selectImageFileName();
	if( !fileName.isEmpty() )
	{
		// extract directory from file name and store it
		m_lastImageDirectory = QFileInfo( fileName ).absolutePath();

		uploadTextureImage( tmu, fileName );
	}
}


/*
========================
uploadTextureImage

 Uploads a texture image to m_textureState.
 It also updates the icon of the load button.
========================
*/
void CTextureWidget::uploadTextureImage( int tmuIndex, const QString & fileName )
{
	// catch this!
	if( tmuIndex < 0 || tmuIndex >= m_validTmus )
		return;

	buttonInfo_t* tmu = &m_tmus[ tmuIndex ];

	// try loading the image
	if( !tmu->image.load( fileName ) )
	{
		// failed!
		QMessageBox::warning( this, CONFIG_STRING_ERRORDLG_TITLE, "Failed to load image!",
			QMessageBox::Ok, QMessageBox::Ok );
		tmu->image = QImage();
	}

	// save image name into tooltip
	tmu->button->setToolTip( fileName );

	//
	// update icon and GL
	//

	// leave some space for the border
	tmu->button->setIconSize( textureIconSize() );

	// upload icon
	QPixmap pixmap( QPixmap::fromImage( tmu->image.scaled( textureIconSize() ) ) );
	tmu->icon = QIcon( pixmap );
	tmu->button->setIcon( tmu->icon );

	// upload texture
	m_textureState->setTexture( tmuIndex, tmu->image );
}


/*
========================
selectImageFileName

 An empty string is returned, if the user selected no image.
========================
*/
QString CTextureWidget::selectImageFileName( void )
{
	//
	// setup filter list
	//
	bool first = true;
	QString filter( "Image Files (" );
	QList< QByteArray > supported = QImageReader::supportedImageFormats();
	foreach( QByteArray a, supported )
	{
		filter.append( QString( "%1*.%2" )
			.arg( first ? QString("") : QString(" ") )
			.arg( QString(a) ) );
		first = false;
	}
	// let the user choose to show all files
	filter.append( ");;All Files (*)" );


	// setup initial directory
	QString initialDir( CONFIG_TEXTURE_DIRECTORY );
	if( m_lastImageDirectory.length() > 0 )
		initialDir = m_lastImageDirectory;


	// select the filename
	QString fileName = QFileDialog::getOpenFileName( this,
		QString( "Open Image File" ), initialDir, filter );

	return fileName;
}


/*
========================
updateSamplerList
========================
*/
void CTextureWidget::updateSamplerList( void )
{
	CUniform u;
	int firstIndex = -1;

	// disable signals
	m_skipSetActiveSampler++;
	m_skipSetSamplerTmu++;

	// clear old state
	m_activeSampler->clear();

	// setup combo box
	int numUniforms = m_uniformState->getActiveUniforms();
	for( int i = 0 ; i < numUniforms ; i++ )
	{
		// filter out invalid types
		u = m_uniformState->getUniform( i );
		if( u.getType() != GL_SAMPLER_2D )
		{
			continue;
		}

		// we got a variable!
		if( firstIndex == -1 )
			firstIndex = i;

		// store name and uniform index!
		m_activeSampler->addItem( u.getName(), QVariant( i ) );
	}

	// upload first value
	if( firstIndex != -1 )
	{
		u = m_uniformState->getUniform( firstIndex );
		m_samplerTmu->setValue( u.getValueAsInt( 0 ) );
	}

	// no variables, not editable!
	m_activeSampler->setEnabled( firstIndex != -1 );
	m_samplerTmu->setEnabled( firstIndex != -1 );

	// enable signals
	m_skipSetActiveSampler--;
	m_skipSetSamplerTmu--;
}


/*
========================
setActiveSampler
========================
*/
void CTextureWidget::setActiveSampler( int active )
{
	if( m_skipSetActiveSampler )
		return;

	m_skipSetActiveSampler++;
	m_skipSetSamplerTmu++;

	// translate index
	int index = uniformIndex( active );

	// upload new value -> download is done via setSamplerTmu()
	CUniform u = m_uniformState->getUniform( index );
	m_samplerTmu->setValue( u.getValueAsInt( 0 ) );

	m_skipSetActiveSampler--;
	m_skipSetSamplerTmu--;
}


/*
========================
setSamplerTmu
========================
*/
void CTextureWidget::setSamplerTmu( int tmu )
{
	if( m_skipSetSamplerTmu )
		return;

	m_skipSetSamplerTmu++;

	// validate sampler
	int index = m_activeSampler->currentIndex();
	if( index >= 0 )
	{
		// translate index
		index = uniformIndex( index );

		// replace uniform data
		CUniform u;
		u = m_uniformState->getUniform( index );
		u.setValueAsInt( 0, tmu );
		m_uniformState->setUniform( index, u );
	}

	m_skipSetSamplerTmu--;
}


/*
========================
checkBilinearFilter
========================
*/
void CTextureWidget::checkBilinearFilter( int checkState )
{
	m_textureState->setBilinearFilter( checkState != Qt::Unchecked );
}


/*
========================
uniformIndex

 converts m_activeSampler indices to m_uniformState indices
========================
*/
int CTextureWidget::uniformIndex( int comboBoxIndex )
{
	return m_activeSampler->itemData( comboBoxIndex ).toInt();
}


