//=============================================================================
/** @file		texturewidget.h
 *
 * Defines the texture management widget class
 *
	@internal
	created:	2007-11-28
	last mod:	2008-01-28

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __TEXTUREWIDGET_H_INCLUDED__
#define __TEXTUREWIDGET_H_INCLUDED__

#include <QtCore/QSignalMapper>
#include <QWidget>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>

#include "config.h"

// forward declarations
class ITextureState;
class IUniformState;


//=============================================================================
//	CTextureWidget
//=============================================================================

/** Widget for manipulating ITextureState.
 * This widget allows the user to load image files into texture mapping unis,
 * and to assign texture units to samplers.
 * It is bound to a ITextureState object, which must live longer than the
 * texture widget. It is also bound to a IUniformState object, which is
 * used to manipulate uniforms of type GL_SAMPLER_2D.
 * \n\n
 * A CTextureWidget can be constructed without a valid OpenGL context, but
 * such acontext is required for initialization of the texture widget.
 * @see ITextureState
 */
class CTextureWidget : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a texture widget.
	 * @param ts ITextureState object to manipulate.
	 * @param us IUniformState object to manipulate.
	 */
	CTextureWidget( ITextureState* ts, IUniformState* us );

	/** Initializes the texture widget.
	 * After this call, the widget is ready to operate.
	 * @pre
	 *  The widget must be constructed with valid interfaces and
	 *  a valid OpenGL context active. If these requrements are not met,
	 *  behavior is undefined.
	 */
	void init( void );

	/** Put the widget into an uninitialized state.
	 * @pre A valid OpenGL context must be active when calling this.
	 */
	void shutdown( void );

public slots:
	/** Forces the texture widget to update it's sampler list.
	 * Clears it current sampler list and loads a new list of the bound
	 * IUniformState object.
	 * @pre The widget must be initialized before this slot can be used.
	 */
	void updateSamplerList( void );

private slots:
	void setActiveSampler( int active );
	void setSamplerTmu( int tmu );
	void selectTexture( int tmu ); // bring up 'load image' dialog
	void checkBilinearFilter( int checkState );

private:

	/** This widget supports up to that many TMUs. */
	enum { MAX_USED_TMUS = CONFIG_MAX_USED_TMUS };

	// helpers
	QGroupBox*	createSamplerWidget( void );
	QGroupBox*	createTmuWidget( void );
	int			uniformIndex( int comboBoxIndex );

	// texture loading helpers
	QString		selectImageFileName( void );
	void		uploadTextureImage( int tmu, const QString & fileName );

	// texture button size helpers
	QSize textureButtonSize( void ) const;
	QSize textureIconSize  ( void ) const;

	// widgets
	QComboBox*		m_activeSampler;
	QSpinBox*		m_samplerTmu; // TMU of the active sampler
	QCheckBox*		m_chkBilinearFilter;

	// mapping from buttons to TMUs
	QSignalMapper*	m_mapper;

	// where to look for images when the 'load image' dialog pops up.
	QString m_lastImageDirectory;

	// signal processing
	int m_skipSetActiveSampler;
	int m_skipSetSamplerTmu;

	// helpers struct for texture buttons.
	typedef struct buttonInfo_s {
		QPushButton*	button;
		QGroupBox*		groupBox;
		QImage			image; // loaded image
		QIcon			icon;  // small version of image
	} buttonInfo_t;

	// texture unit widgets
	buttonInfo_t	m_tmus[ MAX_USED_TMUS ];
	int				m_validTmus; // number of used elements in m_tmus

	ITextureState* m_textureState;
	IUniformState* m_uniformState;
};


#endif	// __TEXTUREWIDGET_H_INCLUDED__
