//=============================================================================
/** @file		scenewidget.cpp
 *
 * Implements CSceneWidget.
 *
	@internal
	created:	2007-11-24
	last mod:	2008-02-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QGridLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QValidator>

#include "application.h"
#include "scene.h"
#include "scenewidget.h"
#include "camera.h"
#include "model.h"
#include "shader.h"


//=============================================================================
//	CSceneWidget implementation
//=============================================================================

// construction
CSceneWidget::CSceneWidget( IScene* scene ) : m_scene( scene )
{
	m_models = NULL;
	m_numModels = 0;
	m_meshModelIndex = -1;
	m_meshModel = NULL;
	m_meshFileName = QString( "" );
    m_vertexDensityLevel=7;

	//
	// setup geometry state widgets
	//
	m_chkUseProgram       = new QCheckBox( "Use GLSL Program" );
	m_chkWireframe        = new QCheckBox( "Wireframe rendering" );
	m_chkBackFaceCulling  = new QCheckBox( "Back Face Culling" );
	m_chkShowOrigin       = new QCheckBox( "Show Origin" );
	m_chkShowNormals      = new QCheckBox( "Show Normals" );
	m_chkShowBoundingBox  = new QCheckBox( "Show Bounding Box" );
	m_chkShowTangents     = new QCheckBox( "Show Tangent Space" );
	m_activeModel         = new QComboBox();
	QLabel* testModelText = new QLabel( "Test Model:" );
	QGroupBox* groupModel = new QGroupBox( "Geometry Processing" );
    m_vertexDensity       = new QLineEdit("7");
	QGridLayout* groupModelLayout = new QGridLayout();
    groupModelLayout->addWidget( testModelText,        0,0, 1,1 );
	groupModelLayout->addWidget( m_activeModel,        0,1, 1,1 );
	groupModelLayout->addWidget( m_chkUseProgram,      1,0, 1,2 );
	groupModelLayout->addWidget( m_chkWireframe,       2,0, 1,2 );
	groupModelLayout->addWidget( m_chkBackFaceCulling, 3,0, 1,2 );
	groupModelLayout->addWidget( m_chkShowOrigin,      4,0, 1,2 );
	groupModelLayout->addWidget( m_chkShowNormals,     5,0, 1,2 );
	groupModelLayout->addWidget( m_chkShowBoundingBox, 6,0, 1,2 );
	groupModelLayout->addWidget( m_chkShowTangents,    7,0, 1,2 );
    groupModelLayout->addWidget( new QLabel("Vertex Density(1,20):"),      8,0, 1,1 );
    groupModelLayout->addWidget( m_vertexDensity,      8,1, 1,1 );
	groupModel->setLayout( groupModelLayout );

	// setup tool tips
	m_chkUseProgram->     setToolTip( "Switch between GLSL and fixed-function pipeline" );
	m_chkWireframe->      setToolTip( "Sets GL_POLYGON_MODE to GL_LINES" );
	m_chkBackFaceCulling->setToolTip( "Enables back face culling." );
	m_chkShowOrigin->     setToolTip( "Draws the world-space origin." );
	m_chkShowTangents->   setToolTip( "Draws the tangent space vectors for each vertex.\nTangent in red, bitangent in green, normal in blue" );
	m_chkShowNormals->    setToolTip( "Draws the normal of each vertex.\nThe color is choosen from the greatest normal component." );
	m_chkShowBoundingBox->setToolTip( "Draws the model's bounding box.\nRed == X axis, green == Y axis, blue == Z axis." );
    m_vertexDensity->     setToolTip( "How many triangles should be drawn");

	//
	// setup projection mode group
	//
	m_btnClearColor       = new QPushButton( "" );
	QLabel* colorBtnText  = new QLabel( "glClearColor" );
	m_fov = new QComboBox();
	m_fov->addItem( QString( "FovY: 10 deg." ), QVariant( (double)10 ) );
	m_fov->addItem( QString( "FovY: 20 deg." ), QVariant( (double)20 ) );
	m_fov->addItem( QString( "FovY: 30 deg." ), QVariant( (double)30 ) );
	m_fov->addItem( QString( "FovY: 40 deg." ), QVariant( (double)40 ) );
	m_fov->addItem( QString( "FovY: 50 deg." ), QVariant( (double)50 ) );
	m_fov->addItem( QString( "FovY: 60 deg." ), QVariant( (double)60 ) );
	m_fov->addItem( QString( "FovY: 70 deg." ), QVariant( (double)70 ) );
	m_fov->addItem( QString( "FovY: 80 deg." ), QVariant( (double)80 ) );
	m_fov->addItem( QString( "FovY: 90 deg." ), QVariant( (double)90 ) );
	m_fov->addItem( QString( "FovY: 100 deg." ), QVariant( (double)100 ) );
	m_fov->addItem( QString( "FovY: 110 deg." ), QVariant( (double)110 ) );
	m_fov->addItem( QString( "FovY: 120 deg." ), QVariant( (double)120 ) );
	m_fov->addItem( QString( "FovY: 130 deg." ), QVariant( (double)130 ) );
	m_fov->addItem( QString( "FovY: 140 deg." ), QVariant( (double)140 ) );
	m_fov->addItem( QString( "FovY: 150 deg." ), QVariant( (double)150 ) );
	m_fov->addItem( QString( "FovY: 160 deg." ), QVariant( (double)160 ) );
	m_fov->addItem( QString( "FovY: 170 deg." ), QVariant( (double)170 ) );
	m_fov->setToolTip( QString( "Sets the vertical field-of-view" ) );
	m_fov->setCurrentIndex( 4 );
	m_projectionMode = new QComboBox();
	m_projectionMode->addItem( QString( "glFrustum" ), QVariant( (int)ICameraState::PROJECT_FRUSTUM ) );
	m_projectionMode->addItem( QString( "glOrtho"   ), QVariant( (int)ICameraState::PROJECT_ORTHO   ) );
	m_projectionMode->setCurrentIndex( 0 );
	QGroupBox* groupProjection = new QGroupBox( "Projection Matrix / Viewport" );
	QGridLayout* groupProjectionLayout = new QGridLayout();
	groupProjectionLayout->addWidget( m_projectionMode,		0,0, 1,1 );
	groupProjectionLayout->addWidget( m_fov,				0,1, 1,1 );
	groupProjectionLayout->addWidget( colorBtnText,			1,0, 1,1 );
	groupProjectionLayout->addWidget( m_btnClearColor,		1,1, 1,1 );
	groupProjection->setLayout( groupProjectionLayout );

	//
	// setup mesh group
	//
	m_btnLoadMesh = new QPushButton( "-" );
	QGroupBox* groupMesh = new QGroupBox( "Mesh File" );
	QGridLayout* groupMeshLayout = new QGridLayout();
	groupMeshLayout->addWidget( m_btnLoadMesh );
	groupMesh->setLayout( groupMeshLayout );

	//
	// setup geometry shader group
	//
	m_labPrimitiveType		= new QLabel();
	m_geometryOutputType	= new QComboBox();
	QLabel* primTypeInText	= new QLabel( "Input Type:" );
	QLabel* primTypeOutText	= new QLabel( "Output Type:" );
	QLabel* relinkWarning   = new QLabel( "NOTE: if these values change,\nyou must re-link the program." );
	m_groupGeometryShader = new QGroupBox( "Geometry Shader" );
	QGridLayout* groupGeometryShaderLayout = new QGridLayout();
	groupGeometryShaderLayout->addWidget( primTypeInText,		0,0, 1,1 );
	groupGeometryShaderLayout->addWidget( m_labPrimitiveType,	0,1, 1,1 );
	groupGeometryShaderLayout->addWidget( primTypeOutText,		1,0, 1,1 );
	groupGeometryShaderLayout->addWidget( m_geometryOutputType,	1,1, 1,1 );
	groupGeometryShaderLayout->addWidget( relinkWarning,        2,0, 1,2 );
	m_groupGeometryShader->setLayout( groupGeometryShaderLayout );
	m_geometryOutputType->addItem( "GL_POINTS",			QVariant( int(GL_POINTS) ) );
	m_geometryOutputType->addItem( "GL_LINE_STRIP",		QVariant( int(GL_LINE_STRIP) ) );
	m_geometryOutputType->addItem( "GL_TRIANGLE_STRIP",	QVariant( int(GL_TRIANGLE_STRIP) ) );
	m_geometryOutputType->setCurrentIndex( 2 ); // default to GL_TRIANGLE_STRIP
	relinkWarning->setToolTip( "This is necessary because the primitive\ntypes are required for linking." );

	// misc widgets
	m_btnResetCamera = new QPushButton( QString( "Reset Camera Positon And Orientation" ) );

	// setup layout
	QGridLayout* layout = new QGridLayout();
	layout->addWidget( groupModel,            0,0, 3,1 );
	layout->addWidget( groupProjection,       0,1, 1,1 );
	layout->addWidget( groupMesh,             1,1, 1,1 );
	layout->addWidget( m_groupGeometryShader, 2,1, 1,1 );
	layout->addWidget( m_btnResetCamera,      3,0, 1,2 );
	setLayout( layout );

	// initialize check box state
	m_chkUseProgram->setCheckState( Qt::Checked );

	// set initial clear color
	QPalette pal = m_btnClearColor->palette();
	pal.setColor( QPalette::Button, QColor(0,0,0) );
	m_btnClearColor->setPalette( pal );

    // initial vertex density
    m_vertexDensity->setValidator(new QIntValidator(-1,100));
    m_vertexDensity->setReadOnly(false);

	// setup signals
	connect( m_chkUseProgram,      SIGNAL(stateChanged(int)),        this, SLOT(checkUseProgram(int)) );
	connect( m_chkWireframe,       SIGNAL(stateChanged(int)),        this, SLOT(checkWireframe(int)) );
	connect( m_chkBackFaceCulling, SIGNAL(stateChanged(int)),        this, SLOT(checkBackFaceCulling(int)) );
	connect( m_chkShowOrigin,      SIGNAL(stateChanged(int)),        this, SLOT(checkShowOrigin(int)) );
	connect( m_chkShowNormals,     SIGNAL(stateChanged(int)),        this, SLOT(checkShowNormals(int)) );
	connect( m_chkShowBoundingBox, SIGNAL(stateChanged(int)),        this, SLOT(checkShowBoundingBox(int)) );
	connect( m_chkShowTangents,    SIGNAL(stateChanged(int)),        this, SLOT(checkShowTangents(int)) );
	connect( m_btnResetCamera,     SIGNAL(clicked(bool)),            this, SLOT(resetCamera(bool)) );
	connect( m_btnClearColor,      SIGNAL(clicked(bool)),            this, SLOT(selectClearColor(bool)) );
	connect( m_btnLoadMesh,        SIGNAL(clicked(bool)),            this, SLOT(loadMesh(bool)) );
	connect( m_activeModel,        SIGNAL(currentIndexChanged(int)), this, SLOT(setActiveModel(int)) );
	connect( m_geometryOutputType, SIGNAL(currentIndexChanged(int)), this, SLOT(setGeometryOutputType(int)) );
	connect( m_projectionMode,     SIGNAL(currentIndexChanged(int)), this, SLOT(setProjectionMode(int)) );
	connect( m_fov,                SIGNAL(currentIndexChanged(int)), this, SLOT(setFov(int)) );
    connect(m_vertexDensity,       SIGNAL(editingFinished()),        this, SLOT(setVertexDensity()));
}

CSceneWidget::~CSceneWidget( void )
{
	SAFE_DELETE_ARRAY( m_models );
}


/*
========================
init

 assumes we have a valid OpenGL rendering context active.
========================
*/
void CSceneWidget::init( void )
{
	m_meshFileName = QString( "" );

	// create destmodels
	m_numModels = 6;
	m_meshModelIndex = 5;
	m_models    = new IModel* [ m_numModels ];
	m_models[0] = IModel::createPoint();
	m_models[1] = IModel::createPlane();
	m_models[2] = IModel::createCube();
	m_models[3] = IModel::createSphere( 32, 64, 1.0f );
	m_models[4] = IModel::createTorus( 32, 24, 1.0f, 0.5f );
	m_models[5] = m_meshModel = IMeshModel::createMeshModel();

	// setup combo box
	for( int i = 0 ; i < m_numModels ; i++ )
	{
		m_activeModel->addItem( m_models[i]->getName() );
	}

	// select a cool initial test model
	m_activeModel->setCurrentIndex( 1 );

	// disable geometry stuff, if that shader is not available
	if( !m_scene->getShader()->isShaderTypeAvailable( IShader::TYPE_GEOMETRY ) )
	{
		m_groupGeometryShader->setEnabled( false );
	}
}


/*
========================
shutdown
========================
*/
void CSceneWidget::shutdown( void )
{
	m_scene->setCurrentModel( NULL );

	// NULL out only, it points into m_models
	m_meshModel = NULL;
	m_meshFileName = QString( "" );

	// destroy testmodels
	for( int i = 0 ; i < m_numModels ; i++ )
	{
		SAFE_DELETE( m_models[ i ] );
	}
	SAFE_DELETE_ARRAY( m_models );
	m_numModels = 0;
}


/*
========================
setActiveModel
========================
*/
void CSceneWidget::setActiveModel( int index )
{
	IModel* mdl = NULL;
	QString ptName( "" );
	int pt = GL_POINTS;

	// validate index
	if( index >= 0 && index < m_numModels )
	{
		mdl = m_models[ index ];
		pt  = mdl->getPrimitiveType();
		ptName = mdl->getPrimitiveTypeName();
	}

	// set new state
	m_scene->getShader()->setGeometryInputType( pt );
	m_labPrimitiveType->setText( ptName );
	m_scene->setCurrentModel( mdl );
}


/*
========================
checkUseProgram
========================
*/
void CSceneWidget::checkUseProgram( int toggleState )
{
	m_scene->setUseProgram( toggleState != Qt::Unchecked );
}



/*
========================
checkWireframe
========================
*/
void CSceneWidget::checkWireframe( int toggleState )
{
	m_scene->setWireframe( toggleState != Qt::Unchecked );
}


/*
========================
checkBackFaceCulling
========================
*/
void CSceneWidget::checkBackFaceCulling( int toggleState )
{
	m_scene->setBackFaceCulling( toggleState != Qt::Unchecked );
}


/*
========================
checkShowOrigin
========================
*/
void CSceneWidget::checkShowOrigin( int toggleState )
{
	m_scene->setShowOrigin( toggleState != Qt::Unchecked );
}


/*
========================
checkShowNormals
========================
*/
void CSceneWidget::checkShowNormals( int toggleState )
{
	m_scene->setShowNormals( toggleState != Qt::Unchecked );
}


/*
========================
checkShowBoundingBox
========================
*/
void CSceneWidget::checkShowBoundingBox( int toggleState )
{
	m_scene->setShowBoundingBox( toggleState != Qt::Unchecked );
}


/*
========================
checkShowTangents
========================
*/
void CSceneWidget::checkShowTangents( int toggleState )
{
	m_scene->setShowTangents( toggleState != Qt::Unchecked );
}


/*
========================
setProjectionMode
========================
*/
void CSceneWidget::setProjectionMode( int index )
{
	// do range check
	if( index < 0 || index >= m_projectionMode->count() )
		return;

	// extract the value
	bool ok = false;
	int value = m_projectionMode->itemData( index ).toInt( &ok );
	if( !ok )
		return;

	// validate the value
	switch( value )
	{
	case ICameraState::PROJECT_FRUSTUM:
		m_scene->getCameraState()->setProjectionMode( ICameraState::PROJECT_FRUSTUM );
		break;

	case ICameraState::PROJECT_ORTHO:
		m_scene->getCameraState()->setProjectionMode( ICameraState::PROJECT_ORTHO );
		break;
	}
}


/*
========================
setGeometryOutputType
========================
*/
void CSceneWidget::setGeometryOutputType( int index )
{
	if( index >= 0 && index < m_geometryOutputType->count() )
	{
		QVariant data = m_geometryOutputType->itemData( index );
		m_scene->getShader()->setGeometryOutputType( data.toInt() );
	}
}


/*
========================
setFov
========================
*/
void CSceneWidget::setFov( int index )
{
	// do range check
	if( index < 0 || index >= m_fov->count() )
		return;

	// extract value
	bool ok = false;
	float value = (float)m_fov->itemData( index ).toDouble( &ok );

	if( ok )
	{
		m_scene->getCameraState()->setFovY( value );
	}
}


/*
========================
loadMesh

 brings up a load file dialog and then loads the selected model file.
========================
*/
void CSceneWidget::loadMesh( bool )
{
	if( m_meshModel == NULL )
		return;

	// setup initial directory
	QString initialDir = m_meshFileName;
	if( initialDir.isEmpty() )
		initialDir = QString( CONFIG_MODEL_DIRECTORY );

	//
	// select a file name
	//
	QString fileName = QFileDialog::getOpenFileName( this,
		QString( "Open .OBJ model" ), initialDir,
		QString( "Wavefront Objects (*.obj);;All Files (*)" ) );
	if( !fileName.isEmpty() )
	{
		// try to load the model.
		if( !m_meshModel->loadObjModel( fileName ) )
		{
			QMessageBox::warning( this, CONFIG_STRING_ERRORDLG_TITLE,
				QString( "Failed to load mesh file %1." ).arg( fileName ) );
		}

		// loading succeeded, save the file name.
		else
		{
			m_meshFileName = fileName;
			m_btnLoadMesh->setText( extractFileNameFromPath( fileName ) );

			// make the mesh active
			if( m_meshModelIndex != -1 )
			{
				m_activeModel->setCurrentIndex( m_meshModelIndex );
			}
		}
	}
}


/*
========================
selectClearColor

 brings up a color selection dialog for glClearColor.
========================
*/
void CSceneWidget::selectClearColor( bool )
{
	QPalette pal = m_btnClearColor->palette();

	// ask for new color
	QColor color = pal.color( QPalette::Button );
	color = QColorDialog::getColor( color, this );

	if( color.isValid() )
	{
		pal.setColor( QPalette::Button, color );
		m_btnClearColor->setPalette( pal );

		// translate color
		vec4_t v(
			float(color.red())   / 255.0f,
			float(color.green()) / 255.0f,
			float(color.blue())  / 255.0f,
			1.0f );

		// update state
		m_scene->setClearColor( v );
	}
}


/*
========================
resetCamera
========================
*/
void CSceneWidget::resetCamera( bool )
{
	m_scene->getCameraState()->resetCamera();
}

void CSceneWidget::setVertexDensity(){
    const QString &text=m_vertexDensity->text();
    // Check density range
    int density=atoi(text.toStdString().c_str());
    if(density>14|| density<1){
        QMessageBox* box = new QMessageBox();
        box->setWindowTitle(QString("Out of range"));
        box->setText(QString("The density should be in range of 1-14. default is 7.\nCurrent Text:\""+text+"\""));
        box->show();

        // restor it.
        m_vertexDensity->setText("7");
        return;
    }
    // set density
    m_vertexDensityLevel=density;

    // rebuild plane and cube;
    IModel* plane= m_models[1];
    IModel* cube = m_models[2];
    m_models[1] = IModel::createPlane();
    m_models[2] = IModel::createCube();
//    delete plane;
//    delete cube;
}

