//=============================================================================
/** @file		application.h
 *
 * Defines main application interfaces and constants.
 *
	@internal
	created:	2007-10-24
	last mod:	2008-01-28

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __APPLICATION_H_INCLUDED__
#define __APPLICATION_H_INCLUDED__


//=============================================================================
// internal compile time configuration
//=============================================================================

#include "config.h"

//
// the OpenGL "cross platform" graphics library.
// -> Must be included after Qt (see above) but BEFORE gl/gl.h
//    This is a big problem, because It must be included
//    before the Qt OpenGL code.
//
#include "glee/GLee.h"

//=============================================================================
//	including headers
//=============================================================================

// HACK HACK HACK:
// if glee.h is included before this, Qt complains:
// qt4/QtGui/qstyleoption.h:130: error: expected identifier before numeric constant
// qt4/QtGui/qstyleoption.h:130: error: expected `}' before numeric constant
// ...
//
#if ( CONFIG_USE_QTGUI != 0 )

// this is for universal use and should work with all Qt versions.
#include <QtGui/QtGui>

#else

// this is the 'fast compile' version, but seems to cause problems with some Qt versions.
#include <QtCore/QMetaType>
#include <QtCore/QDataStream>
#include <QtCore/QString>
#include <QtGui/QTextEdit>
#include <QtGui/QComboBox>
#include <QtGui/QMainWindow>
#include <QtGui/QApplication>
#endif


//
// the OpenGL "cross platform" graphics library.
// -> Must be included after Qt (see above) but BEFORE gl/gl.h
//    This is a big problem, because It must be included
//    before the Qt OpenGL code.
//
#include "glee/GLee.h"


//=============================================================================
//	custom vertex attributes
//=============================================================================

/** Vertex attribute locations of a GLSL program.
 *
 * This is a list of all known/supported vertex attributes used by
 * the application.
 */
class VertexAttribLocations
{
public:
	/** Constructs the attribute locaion from given values.
	 * It defaults to -1, which means 'not present'.
	 * @param Tangent	Tangent attrubute.
	 * @param Bitangent	Bitangent attrinute.
	 */
	VertexAttribLocations( int Tangent=-1, int Bitangent=-1 )
		: tangent(Tangent), bitangent(Bitangent)
	{
	}

	/** Returns nonzero if the objects store the same values, otherwise zero.
	 */
	inline int operator==( const VertexAttribLocations & other ) const
	{
		return	( this->tangent		== other.tangent ) &&
				( this->bitangent	== other.bitangent );
	}

	/** Returns zero if the objects store the same values, otherwise nonzero.
	 */
	inline int operator!=( const VertexAttribLocations & other ) const
	{
		return !( *this == other );
	}

	// tangent space basis vectors
	int tangent;	///< tangent attrinute location.
	int bitangent;	///< bitangent attribute location.
};


//=============================================================================
//	useful macros
//=============================================================================

/** Deletes an object, if not NULL.
 * @warning Do not use this for arrays!
 */
#define SAFE_DELETE(p)			do{  if( (p) != NULL ) { delete    (p); (p)=NULL; } } while(0)

/** Deletes an array of objects, if not NULL.
 * @warning Only use this for arrays!
 */
#define SAFE_DELETE_ARRAY(p)	do{  if( (p) != NULL ) { delete [] (p); (p)=NULL; } } while(0)


//=============================================================================
//	useful functions
//=============================================================================

/** Extracts the file name from a path and filename string.
 * This can be used to deal with filenames that do not
 * refer to an exising file. \n
 * Examples: \n
 * "/etc/fstab" -> "fstab" \n
 * "C:\Windows\Explorer.exe" -> "Explorer.exe"\n
 * @param path Path to a file.
 * @return File name without path.
 */
extern QString extractFileNameFromPath( const QString & path );



#endif	// __APPLICATION_H_INCLUDED__

