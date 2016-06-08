//=============================================================================
/** @file		config.h
 *
 *	Defines global compile time configuration constants.
 *
    @internal
    created:	2007-10-28
    last mod:	2008-02-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer
    For details, see main.cpp or COPYING.

=============================================================================*/

#ifndef __CONFIG_H_INCLUDED__
#define __CONFIG_H_INCLUDED__


//=============================================================================
//	version identification
//=============================================================================

/** Version identification string */
#ifndef _DEBUG
#define VERSION_STRING	"1.02, compiled:  " __DATE__ "  " __TIME__
#else
#define VERSION_STRING	"1.02, debug build, compiled:  " __DATE__ "  " __TIME__
#endif


//=============================================================================
//	compatibility
//=============================================================================

/** Use the QtGui/QtGui include files. If this is zero,
 * the required files must be included manually. Setting this to 1 should
 * work with all Qt versions but significantly slows down compilation time.
 * Setting this to zero will compile faster, but might not work
 * with some Qt versions.
 */
#define CONFIG_USE_QTGUI	1


//=============================================================================
//	misc
//=============================================================================

#define CONFIG_SHADER_DIRECTORY		"shaders/"	///< Where to look for shader source files
#define CONFIG_MODEL_DIRECTORY		"models/"	///< Where to look for .OBJ model files
#define CONFIG_TEXTURE_DIRECTORY	"textures/"	///> Where to look for texture iamge files
#define CONFIG_EDITOR_FONT_NAME		"Courier"	///< Editor font
#define	CONFIG_TAB_SIZE				4			///< One tab quals that many spaces
#define CONFIG_REFRESH_INTERVAL		10			///< 100 fps, periodic screen refesh in ms
#define CONFIG_MAX_USED_TMUS		4			///< number of texture mapping units accessable by CTextureWidget

/** Commet this out to disable geometry shader support */
#define CONFIG_ENABLE_GEOMETRY_SHADER

/** Font size for the editor.
 *  10 is hard to read on linux.
 */
#ifdef WIN32
#define CONFIG_EDITOR_FONT_SIZE		10
#else // linux, mac
#define CONFIG_EDITOR_FONT_SIZE		12
#endif


//=============================================================================
//	string constants
//=============================================================================

#define CONFIG_STRING_APPLICATION_TITLE			"Shader Maker"				///< Window title string
#define CONFIG_STRING_FILE_UNTITLED				"untitled"					///< Default new file name
#define CONFIG_STRING_ERRORDLG_TITLE			"... always bad news ..."	///< Error message titles
#define CONFIG_STRING_SHOULD_SAVE_FILE_TITLE	"Save Changes"				///< Message box title
#define CONFIG_STRING_SHOULD_SAVE_FILE			"The %1 '%2' has been modified.\nSave changes?"	///< Message box question

/** This text is displayed in the 'about' dialog. */
#define CONFIG_STRING_ABOUT_BOX_TEXT \
	"Shader Maker, a cross-platform GLSL editor.\n" \
    "Copyright (C) 2007 ad infinitum Markus Kramer & G. Zachmann.\n" \
    "\n" \
    "This Software comes with ABSOLUTELY NO WARRANTY; for details see the file COPYING.\n" \
    "This is free software, and you are welcome to redistribute it " \
    "under certain conditions; see the file COPYING for details.\n" \
	"\n" \
	"This software can be downloaded at\n" \
	"http://cg.in.tu-clausthal.de/teaching/shader_maker/\n" \
	"\n" \
	"Version: " VERSION_STRING "\n"

#endif	// __CONFIG_H_INCLUDED__
