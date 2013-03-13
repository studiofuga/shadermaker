//=============================================================================
/** @file		glwidget.h
 *
 * Defines the OpenGL rendering widget.
 *
	@internal
	created:	2007-11-30
	last mod:	2008-03-30

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __GLWIDGET_H_INCLUDED__
#define __GLWIDGET_H_INCLUDED__

#include <QtCore/QTime>
#include <QtOpenGL/QGLWidget>

// forward declarations
class ICameraState;


//=============================================================================
// CGLWidget
//=============================================================================

/** OpenGL 2.0 rendering widget.
 * This widget is used to create and manage an OpenGL 2.0 context.
 * It is also responsible for capturing mouse and keyboard events and
 * for their processing. It interacts with a ICameraState object by 
 * manipulating it's rotation and translation properties.
 * The widget must be shown to initialize the renderng context.
 * \n\n
 * The widet emits render() events in a regular time interval.
 */
class CGLWidget : public QGLWidget
{
	Q_OBJECT
public:
	/** Constructs a GL widget.
	 * @param format Pixel format for the OpenGL context.
	 * @param cs ICameraState object to manipulate.
	 */
	CGLWidget( const QGLFormat & format, ICameraState* cs );
	virtual ~CGLWidget( void ); ///< Destructor.

	/** Check, wether the OpenGL context is active.
	 * If the widget owns a valid OpenGL 2.0 rendering context,
	 * this method returns True. Otherwise it returns False.
	 * @return Wether a valid OpenGL 2.0 context is active.
	 */
	bool isSuccessfullyInitialized( void ) const { return m_initSucceeded; }


	/** Returns a driver info string containing.
	 * The string is intended to be displayed to the user.
	 * The OpenGL context must be initialized to use this call.
	 */
	QString getDriverInfoString( void ) const;

signals:;

	/** Periodic render event.
	 * The user of this widget can be notified, that it is time to
	 * redraw it's scene. When this signal is processed, the GL widget
	 * has already set the current viewport parameters, so yout can
	 * access them with glGetFloatv( GL_VIEWPORT, ... ).
	 */
	void render( void );

private:

	// QWidget inherited
	QSize minimumSizeHint( void ) const;
	QSize sizeHint( void ) const;

	// QGLWidget inherited
	void paintGL( void );
	void initializeGL( void );
	void resizeGL( int width, int height );

	// event handling
	void  timerEvent( QTimerEvent* timer );
	void  mouseMoveEvent( QMouseEvent* event );
	void  keyPressEvent( QKeyEvent* event );

	// parses GL_VERSION and checks wether we have the requested version.
	bool validateDriverVersion( int majorRequired, int minorRequired );

	// FPS
	void drawFPS( void );
	int   m_fpsCounter;     // counter for this period
	int   m_fpsValue;       // dispalyed to the used
	int   m_fpsLastPeriod;  // time point of last update
	QTime m_fpsTimer;

	// caught on resizeGL()
	QSize m_viewportSize;

	// store last position to calc deltas
	QPoint m_lastMousePosition;

	// wether OpenGL 2.0 and shader functions are available.
	bool m_initSucceeded;

	// the widgets transforms input events to camera transformations
	ICameraState* m_cameraState;
};


#endif	// __GLWIDGET_H_INCLUDED__


