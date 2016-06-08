//=============================================================================
/** @file		universalslider.cpp
 *
 * Implements the CUniversalSlider widget
 *
	@internal
	created:	2007-11-16
	last mod:	2007-11-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#include <QGridLayout>
#include <QLabel>
#include "universalslider.h"


//=============================================================================
//	CUniversalSlider implementation
//=============================================================================

// construction
CUniversalSlider::CUniversalSlider( const QString & componentName )
{
	// disable signal processing at startup
	m_skipUpdateRange = 1;
	m_skipUpdateValue = 1;
	m_skipUpdateSlider = 1;

	// the slider has the same range for every type. the actual value
	// must be interpolated between minimum and maximum.
	// -> QSlider can't deal with doubles
	m_sliderRange = 100;

	// initialize the widgets
	createWidgets( componentName );
	setContentBool( 0 );

	// setup connections
	connect( m_slider,  SIGNAL(valueChanged(int)),    this, SLOT(retranslateSliderValue(int)) );
	connect( m_minimum, SIGNAL(valueChanged(double)), this, SLOT(updateMinimum(double)) );
	connect( m_maximum, SIGNAL(valueChanged(double)), this, SLOT(updateMaximum(double)) );
	connect( m_value,   SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)) );

	// enable signal processing
	m_skipUpdateRange = 0;
	m_skipUpdateValue = 0;
	m_skipUpdateSlider = 0;
}


/*
========================
createWidgets
========================
*/
void CUniversalSlider::createWidgets( const QString & name )
{
	QGridLayout* layout = new QGridLayout();

	// name
	QLabel* label = new QLabel( name );
	layout->addWidget( label, 0, 0 );
	layout->setColumnStretch( 0, 1 );

	// value spin box
	m_value = new QDoubleSpinBox();
	layout->addWidget( m_value, 0, 1 );
	layout->setColumnStretch( 1, 2 );

	// minimum
	m_minimum = new QDoubleSpinBox();
	layout->addWidget( m_minimum, 0, 2 );
	layout->setColumnStretch( 2, 2 );

	// slider
	m_slider = new QSlider( Qt::Horizontal );
	m_slider->setRange( 0, m_sliderRange );
	layout->addWidget( m_slider, 0, 3 );
	layout->setColumnStretch( 3, 15 );

	// maximum
	m_maximum = new QDoubleSpinBox();
	layout->addWidget( m_maximum, 0, 4 );
	layout->setColumnStretch( 4, 2 );

	setLayout( layout );
}


/*
========================
setContentBool
========================
*/
void CUniversalSlider::setContentBool( bool value )
{
	setFloatMode( false );
	setRangeFixed( true );
	setDataRange( 0, 1  );
	setSlider( 0, 1, value );
}


/*
========================
setContentInt
========================
*/
void CUniversalSlider::setContentInt( int minimum, int maximum, int value )
{
	setFloatMode( false );
	setRangeFixed( false );
	setDataRange( minimum, maximum );
	setSlider( minimum, maximum, value );
}


/*
========================
setContentFloat
========================
*/
void CUniversalSlider::setContentFloat( double minimum, double maximum, double value )
{
	setFloatMode( true );
	setRangeFixed( false );
	setDataRange( minimum, maximum );
	setSlider( minimum, maximum, value );
}


/*
========================
getContentBool
========================
*/
bool CUniversalSlider::getContentBool( void )
{
	// map floating point to boolean range...
	return ( m_value->value() ) > 0.0 ? true : false;
}


/*
========================
getContentInt
========================
*/
int CUniversalSlider::getContentInt( void )
{
	return static_cast<int>( m_value->value() );
}


/*
========================
getContentFloat
========================
*/
double CUniversalSlider::getContentFloat( void )
{
	return m_value->value();
}


//======================
/** Sets the range of the slider widget.
 * It sets a built-in range limit for the minimum and maximum widgets.
 * Then it sets the values for those widgets.
 * Finally it sets the range of the slider widget.
 * @param minimum New minimum.
 * @param maximum New maximum.
 */
//======================
void CUniversalSlider::setDataRange( double minimum, double maximum )
{
	// possible ranges
	m_minimum->setRange( -999999, +999999 );
	m_maximum->setRange( -999999, +999999 );

	m_minimum->setValue( minimum );
	m_maximum->setValue( maximum );
	m_value  ->setRange( minimum, maximum );
}


//======================
/** Sets the slider widget to a given value.
 * Maps the given floating point range to the slider's integer range.
 * @param minimum Minimum for mapping
 * @param maximum Maximum for mapping.
 * @param value The value to set the slider to.
 */
//======================
void CUniversalSlider::setSlider( double minimum, double maximum, double value )
{
	// prevent indirect recursions...
	if( m_skipUpdateSlider )
		return;

	double rangeDiff = maximum - minimum;

	m_skipUpdateSlider++;

	if( rangeDiff > 0.0 )
	{
		double valueDiff = value - minimum;
		double percent = double(m_sliderRange) * valueDiff / rangeDiff;
		m_slider->setValue( static_cast<int>( percent ) );
	}

	m_skipUpdateSlider--;
}


/*
========================
setRangeFixed

 enables/disables minimum/maximum spin boxes
========================
*/
void CUniversalSlider::setRangeFixed( bool enable )
{
	m_minimum->setEnabled( !enable );
	m_maximum->setEnabled( !enable );
}


/*
========================
setFloatMode

 changes precision of the double spin boxes
========================
*/
void CUniversalSlider::setFloatMode( bool enable )
{
	int precision = 0;
	double stepSize = 1.0;

	if( enable )
	{
		precision = 3;
		stepSize = 0.1;
	}

	m_minimum->setDecimals( precision );
	m_maximum->setDecimals( precision );
	m_value  ->setDecimals( precision );

	m_minimum->setSingleStep( stepSize );
	m_maximum->setSingleStep( stepSize );
	m_value  ->setSingleStep( stepSize );
}


/*
========================
updateRange
========================
*/
void CUniversalSlider::updateMinimum( double value ) // wrapper
{
	updateRange( value, m_maximum->value() );
}

void CUniversalSlider::updateMaximum( double value ) // wrapper
{
	updateRange( m_minimum->value(), value );
}

void CUniversalSlider::updateRange( double minimum, double maximum )
{
	if( m_skipUpdateRange != 0 )
		return;

	m_skipUpdateRange++;

	// remember current value
	double value = m_value->value();

	// adjust slider and range of m_value
	setDataRange( minimum, maximum );
	setSlider( minimum, maximum, value );

	// use original value without rounding error.
	updateValue( value );

	m_skipUpdateRange--;
}


/*
========================
retranslateSliderValue

 reads m_slider and calls updateValue().
 see also setSlider().
========================
*/
void CUniversalSlider::retranslateSliderValue( int value )
{
	double minimum = m_minimum->value();
	double maximum = m_maximum->value();
	double alpha = (static_cast<double>(value)) / double(m_sliderRange);

	// interpolate between minimum and maximum using the slider value
	double dvalue = maximum * alpha + minimum * (1.0 - alpha);

	// do not read back
	m_skipUpdateSlider++;

	// set actual value
	updateValue( dvalue );

	m_skipUpdateSlider--;
}


/*
========================
updateValue

 This is called by m_value and m_slider.

 It is also called recursively, because both objects must
 represent the same value.

 Currently, when changing the range, the contentsChanged
 signal is emitted twice, first time with a rounding error
 caused by the slider, second time with the exact value used before.
========================
*/
void CUniversalSlider::updateValue( double value )
{
	if( m_skipUpdateValue )
		return;

	// update slider
	m_skipUpdateValue++;
	setSlider( m_minimum->value(), m_maximum->value(), value );
	m_value->setValue( value );
	m_skipUpdateValue--;

	emit contentsChanged();
}
