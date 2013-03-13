//=============================================================================
/** @file		universalslider.h
 *
 *	Defines a multi-type multi-editable slider widget class.
 *
	@internal
	created:	2007-11-16
	last mod:	2007-11-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __UNIVERSALSLIDER_H_INCLUDED__
#define __UNIVERSALSLIDER_H_INCLUDED__

#include <QWidget>
#include <QDoubleSpinBox>
#include <QSlider>


//=============================================================================
//	CUniversalSlider
//=============================================================================

/** A multi-type multi-editable slider widget.
 * This widget can be used to edit values of different types
 * with different/dynamic ranges in several way.
 * It supports boolean, integer and floating point values.
 * It provides a slider, which slides between a minimum and a maximum,
 * which can be defined by the user. The values can also be changed directly
 * with a spin box.
 *
 * @warning
 *   Because QDoubleSpinBox uses doubles, but QSlider uses integers,
 *   there might appear some precision problems.
 */
class CUniversalSlider : public QWidget
{
	Q_OBJECT
public:
	/** Constructs a slider object with a given name.
	 * @param componentName Name of the value to edit.
	 */
	CUniversalSlider( const QString & componentName = QString() );

	/** Sets the slider to boolean type.
	 * Boolean types range only from 0 (false) tp 1 (true).
	 * @param value Initial slider value.
	 */
	void setContentBool ( bool value );

	/** Sets the slider to integer type.
	 * @param value Initial slider value.
	 * @param minimum Minimum slider value.
	 * @param maximum Maximum slider value.
	 */
	void setContentInt  ( int minimum, int maximum, int value );

	/** Sets the slider to floating point type.
	 * @param value Initial slider value.
	 * @param minimum Minimum slider value.
	 * @param maximum Maximum slider value.
	 */
	void setContentFloat( double minimum, double maximum, double value );

	/** Get the current value as boolean.
	 * If the current type is not boolean, the current value is converted
	 * into false if <= 0 and true otherwise.
	 *
	 * @return The boolean value stored in the slider.
	 */
	bool getContentBool( void );

	/** Get the current value as integer.
	 * @return The integer value if the slider.
	 */
	int getContentInt( void );

	/** Get the current value as floating point.
	 * @return the floating point vlaue of the slider.
	 */
	double getContentFloat( void );

signals:
	/** Emitted whenever the value of the slider changes.
	 * Becuase the slider can have different types, the notified
	 * object must query the slider value in the desired type by itself.
	 * @warning
	 *  It might happen, that this signal is emitted twice, when the
	 *  user changes minimum or maximum values of the slider.
	 *  This is caused by the rounding error prevention code.
	 */
	void contentsChanged( void );

private slots:
	void updateValue( double value );
	void updateRange( double minimum, double maximum );
	void updateMinimum( double value );
	void updateMaximum( double value );
	void retranslateSliderValue( int value );

private:

	// construction helper
	void createWidgets( const QString & name );

	// state changes
	void setFloatMode( bool enable );
	void setRangeFixed( bool fixed );
	void setDataRange( double minimum, double maximum );
	void setSlider( double minimum, double maximum, double value );

	// internal signal processing filters
	// -> if zero, the signal can be processed.
	// -> used with ++ and -- for recursions
	int m_skipUpdateRange;
	int m_skipUpdateValue;
	int m_skipUpdateSlider; // setSlider called by retranslateSlider

	// widgets
	QDoubleSpinBox*	m_value;
	QDoubleSpinBox*	m_minimum;
	QDoubleSpinBox*	m_maximum;
	QSlider*		m_slider;

	// range of m_slider
	int m_sliderRange;
};


#endif	// __UNIVERSALSLIDER_H_INCLUDED__
