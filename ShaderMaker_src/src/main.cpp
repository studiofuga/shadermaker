//=============================================================================
/** @file main.cpp

    Implements the application's entry point main().

    @internal
    created:	2007-10-22
    last mod:	2008-02-27

    Shader Maker - a cross-platform GLSL editor.
    Copyright (C) 2007-2008 Markus Kramer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

=============================================================================*/

#include <QApplication>
#include <QMessageBox>

#include "application.h"
#include "programwindow.h"

/** @mainpage Documentation / User's manual

This is the documentation of Shader Maker, a free, cross-platform GLSL editor.
It contains the documentation of the source code and some useage examples.

The goal of this application is NOT to be the most powerful shader editor available,
but to be simple enough to learn GLSL. Most of the functionality of the editor should
be self explaining, however there are some details that might not be obvious.
Becasue of that, this manual explains how to use all tools of the editor.

Also note that you should still read the shading language specification.
You should also be familiar with OpenGL, or you will have some
difficulties understanding the examples.
If not explicitly said, all OpenGL state that is not
editable with the editor is left in it's default state. These states can be
looked up in the OpenGL 2.0 specification. Both specifications are available
at http://www.opengl.org .

Table of contents: \n
- @subpage basicusage
- @subpage firstshader
- @subpage textures
- @subpage lighting
- @subpage uniforms
- @subpage geometryshader
- @subpage notes_tangents
- @subpage notes_objmodel
- @subpage known_issues
- @subpage literature
*/


/** @page known_issues Known issues
Here is a list of known issues:

- On Mac OS the options 'Switch to SDI view' and 'Switch to MDI view' freeze the program.
  The reason is unknown, because the feature workes well on Windows and Linux, but it seems
  that a bug in Qt is responsible. If you encounter this problem, you can hard-code your
  preferred view by editing the method CEditor::init() in editor.cpp.

- On Mac OS and Windows XP there is a problem with button colors.
  Several buttons in the GUI bring up a color selection dialog when clicked.
  The selected color is then used as the button's background color.
  Unfortunately, this does not work on some platforms. The buttons are drawn
  with the system's default colors. This appears on Mac OS and Windows XP. On Windows XP,
  this problem appears only when using the default theme. When switching to the 'classic'
  theme, the buttons are drawn correctly. The behaviour on Windows Vista is unknown and not tested.
  The reasons is unknown, but seems to be a bug in Qt or in the underlying operating system.
  A possible workaround whould be to use a built-in style of Qt, where Qt draws everything by itself.
*/

//=============================================================================
//	Basic Useage page
//=============================================================================

/** @page basicusage Basic use
Here you can learn the basic use of the shader editor.

After starting the editor, you should see two windows. On the left side, a window
with many controls and a white quad on a black background. On the right side, you
should see a text editor window with some text in it.

Let's first look at the left window. It contains controls to modify the state
of OpenGL and the active shader. You can see several tabs, the active tab is
titled 'Scene'. The other tabs are of no interest at the moment.

The 'Scene' tab is the most important tab of the editor.
Here you can manipulate the state related to rendered the scene.
It includes the test model, projection matrix, etc.

The first thing to learn is how to navigate the scene. This is useful, if you want
to take a closer look at your shader. If you click with the left mouse button
into the rendering window and move the mouse (while still holding the left mouse button
pressed), you can rotate the test model around it's origin. You can do the same
with the right and middle mouse buttons. The right mouse button let you move
the test model towards and away from the camera position. Middle button moves the
test model parallel to the projection plane. If you 'lost' you test model, you can
use the 'Reset Camera Position and Rotation' button on the bottom of the scene widget.
As it is labeled, it resets the model to it's default position and rotation.
An alternate way to reset the camera is to hit the ESC-key when the rendering window
has the input focus.

Always looking at a quad is boring. If you want to see a more intresting test model,
you can use the 'Test Model' combo box to select another one. If you selected the
'Mesh' model, you won't see anything. This is because you first have to load a model
from a file. Loading models can be done by clicking the button in the 'Mesh File' group
box. It brings up a dialog, whre you can select a model in the .OBJ format. After loading,
the model is scaled to fit into the unit cube.
Note that the shader editor only supports a subset of the .OBJ file format, but
it should be enough for our purpose...

In the 'Projecton Matrix / Viewport'  group box, you can modify OpenGL's projection
matrix. You can set a custom field of view, if you do not like the default. The value
'Auto' selects a vertical FOV of 50 degree and a horizontal FOV relative to the
window's aspect ratio. This makes sure that the test model is not deformed, meaning
that a shpere is always drawn as a sphere and not deformed to an ellipsoid.
Finally, you can select the color that is used in the glClearColor command.
In short, you can select the 'background color'

The 'Geometry Processing' groub box contains several check boxes with
rendering state flags.

- 'Use GLSL Program' lets you switch between the fixed function pipeline and
  a your GLSL program.

- 'Wireframe' should be self explaining...

- 'Back Face Culling' enables OpenGL's back face culling. To see what is does,
  you can look at the cube model with wireframe rendering enabled.

- 'Show origin' draws a line for each of the main axes of the world space
  coordinate system. These axes are colored red (X), green (Y) and blue (Z) and
  point into the positive directions.

- 'Show normals' visualizes the vertex normals of the test model.

- 'Show Bounding Box' draws a bounding box around the test model.

- 'Show Tangent Space' draws a the tangent space vectors of every vertex. The
  tangent is drawn in red, the bitangent in green and the normal in blue.

The 'Geometry Shader' group box deals with special geometry shader state and
is not explained here. See the geometry shader tutorial for an explanation.

Now you are ready to write @ref firstshader.
*/


//=============================================================================
//	The first shader page
//=============================================================================

/** @page firstshader The first shader
In this tutorial you learn how to write GLSL code and test it.

After starting the shader editor, you can the editor window with some source code in it.
There is one window for each shader type: the vertex shader, the fragment shader and
(if your OpenGL implementation supports it) the geometry shader. In this tutorial you
can learn how to write vertex and fragment shader. Geometry shaders are handled in a
separate tutorial.

After a quick look at the vertex shader, you will realize that this shader does nothing
taht's very intresting, it simply passes trough it's input an transforms the vertex poisition.

If you replace the line

gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

with

gl_Position = gl_Vertex;

then you should see that this transformation is quite important.
But wait, you see no changes, right? That's because you actually didn't change
the current program object! Whenever you think you are ready to test
your shader, you must first compile and link your source code by clicking 
the 'Compile and Link' button ( or hit F5 ) to update the program object.
Simply remember 'hit F5 before testing'.


As another example you can compute gl_FrontColor with:

gl_FrontColor = vec4( 1.0, 1.0, 1.0, 0.0 ) - gl_Color;

Now you should see the model with inverse colors.

When you make a mistake and try to compile your code, then the editor
switches to the 'Log' tab. Here you can find informations about the
compile and link process and read error messages. When compilation succeeds,
you can also find a list of active uniforms and vertex attributes. This is
mainly for informational purpose, but might be useful in some situations.


Now let's go on to the fragment shader. Here you can compute a pixel color by
processing the fragment's attributes. The initial fragment shader passes through
the interpolated results of the vertex shader. To get an idea what the fragment
does, try the following:

gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );

Now you should the the test model in red, without any lighting, etc.
The next example darkens the test model:

gl_FragColor = gl_Color * 0.5;

To see the differences between the original color an the darkened color, you
can uncheck the 'Attach to program' check-box ( and then recompile the program ).
With this check box, you can (de-)activate shaders that you do not need in you program.
For example, the geometry shader is always deactivated this way by default.

If you want do develop your program with a separate window for each shader type,
you can use the menu element 'View -> Switch to SDI view'.
Note that you must save your work before using this option.

The next step is to learn @ref textures.
*/


//=============================================================================
//	Notes about tangent vectors
//=============================================================================

/** @page notes_tangents Notes about tangent vectors.
This explains what tangent vectors are good for.

In some cases it might be useful to compute things not in model-space coordinates
but in texture space coordinates. For example, if you look up a vector in a texture map,
this vector has usually an orientation relative to the projection plane.
If you rotate this triangle that uses this vector, the vector must be rotated as well.
The problem is to find the rotation matrix.

This is where tangent vectors come into play.
The shader editor provides two additioal vertex attributes:

attribute vec3 attrTangent;   \n
attribute vec3 attrBitangent; \n

These tangent vectors define an orthogonal coordinate system in texture space.
They are created for each vertex by using the positions and texture coordinates
of the incident triangle. The 'attrTangent' vector represents the positive 's' axis of the
2D texture coordinates in the 3D triangle and the 'attrBitangent' represents the positive
't' axis of the 2D texture coordinates in the 3D triangle.

You can visualize the tangent vectors by checking the 'Show tangent spaqce' check-box
in the scene tab. The tangent is drawn in red, the bitangent in green and the vertex normal
in blue.

The tangent, bitangent and normal attributes of a vertex define an orthonormalbasis.
You can use this basis to transform vectors out of the texture space coordinate system
( also called tangent space ) to the model space coordinate system in which your test model is defined.
If you want to transform a model space vector into tangent space,
you must multiply that vector with the transpose of this matrix.

To create a matrix, that transforms from tangent to model space, you can do the following:

mat3 tangentToModelMatrix; \n
tangentToModelMatrix[0] = attrTangent; \n
tangentToModelMatrix[1] = attrBitangent; \n
tangentToModelMatrix[2] = gl_Normal; \n

and transforming a tangent space vector is done with a multiplication:

vec3 tangentVector = ... \n
vec3 modelVector = tangentToModelMatrix * tangentVector; \n

*/


//=============================================================================
//	Notes about the .OBJ loader
//=============================================================================

/** @page notes_objmodel Notes about the .OBJ loader
The integrated .OBJ loader supports loading of Wavefront .OBJ files.
It supports only a subset of the .OBJ file format, but even the shader editor itself
does not support complex models. OBJ models are interpreted as a set of faces
and therefore groups of faces are ignored. That leads to the problem, that it is not
possible to load material files, because the loader can't decide which of the several
materials in a .MTL file to use. This applies to colors and ( even worse ) to textures.
Although the .OBJ format supports texture coordiantes, these coordinates point into a
texture map specific to the bodygroup to which a vertex belongs. This leads to strange
results when applying textures to that model in the shader editor.

If your model does not include texture coordinates, these coordinates are automatically
generated by sphere mapping. The vertex positions are projected on the unit sphere and then
are mapped to a texture that is 'wrapped' around that sphere. This results in poor quality,
but it's still better than having no texture coords at all...

The same counts for normals. When a model misses vertex normals, these are generated by
calculating the surface normals of all incident triangles, adding them and then normalizing
the result. This leads to normals with a 'smooth' lighting effect.

Because different models have different bounding volumes, the model's vertex coords are
scaled and offsetted to fit into a cube from (-1,-1,-1) to (+1,+1+1) with the model's
bounding box center set to the origin. This ensures that the model fits into the view.

Vertex colors are created from the transformed vertex positions.
The coordinates transformed position are taken to their absolute values
and the result is subtracted from 1.0. This leads to a vector in an inverted RGB cube.

*/

//=============================================================================
//	Used software and literature
//=============================================================================

/** @page literature Used software and literature
To create this software, the following sources where used:

Software (including their documentation):
- Qt 4.2 and 4.3 (http://www.trolltech.com/qt/)
- the GLee library (http://elf-stone.com/glee.php)
- NVIDIA OpenGL SDK (http://www.nvidia.com/)
- DOXYGEN (http://www.stack.nl/~dimitri/doxygen/)
- Microsoft Visual Studio .NET 2003 (http://www.microsoft.com/)

Literature:
- Literature for lecture 'Computergraphic 1' (http://zach.in.tu-clausthal.de/teaching/cg1_0607/index.html)
- Literature for lecture 'Computergraphic 2' (http://zach.in.tu-clausthal.de/teaching/cg2_07/index.html)
- OpenGL 2.0 specification (http://www.opengl.org/documentation/specs/)
- OpenGL Shading Language 1.20 specification (http://www.opengl.org/documentation/specs/)
- Tutorials at http://www.gamedev.net
*/


//=============================================================================
//	Unix style signal handling
//=============================================================================

// enable signal handling code here
#if defined(__linux__) || defined(__APPLE__)

#include <signal.h>


/*
========================
signalHandler

 aborts the program with an error message
========================
*/
static void signalHandler( int sigNum )
{
	//
	// convert int to string
	//
	char sigName[ 32 ];
	switch( sigNum )
	{
	case SIGSEGV: strcpy( sigName, "SIGSEGV" ); break;
	default:
		sprintf( sigName, "%d", sigNum );
		break;
	}

	// print to stderr
	fprintf( stderr, "Caught signal: %s\n", sigName );
	fflush( stderr );

	//
	// display a message to the user
	//
	QMessageBox::warning( NULL, QString( CONFIG_STRING_ERRORDLG_TITLE ),
		QString( "Caught signal %1.\nIf you were linking the shader, it is "
				 "very likely that your OpenGL driver crashed this application." ).
				 arg( QString( sigName ) ) );

	qApp->exit( -1 );

	// abort the process.
	exit( -1 );
}


static void setupSignalHandler( void )
{
	signal( SIGSEGV, signalHandler );
}


#else

//========================
/** Installs signal handler functions.
 * Currently it installs a signal handler for SIGSEGV.
 * All handled signals cause the application to display an error message
 * dialog to the user and then abort the process.
 * This function does nothing in the win32 version.
 */
//========================
static void setupSignalHandler( void )
{
	// this is the win32 version.
}

#endif


//=============================================================================
//	stateless support routines.
//=============================================================================

/*
========================
extractFileNameFromPath
========================
*/
QString extractFileNameFromPath( const QString & path )
{
	// scan for path separators
	int idx = path.lastIndexOf( "/" );
	if( idx == -1 )
		idx = path.lastIndexOf( "\\" );

	// if we have a path
	if( idx != -1 )
	{
		return path.right( path.length() - idx - 1 );
	}

	// no path separators found, assume a file name.
	return path;
}


//=============================================================================
//	entry point
//=============================================================================


/**  C style application entry point.
 * Creates a QApplication and a CProgramWindow object and runs them.
 * Calls init() and shutdown() on the program window.
 *
 * @param argc    argument count
 * @param argv    arguments
 *
 * @return
 *   exit code
 **/
int main( int argc, char* argv[] )
{
	int code = -1;

	// setup signal handler on UNIX style systems
	setupSignalHandler();

	// init application object
	QApplication app( argc, argv );

	// init Qt resource system
	Q_INIT_RESOURCE( images );

	app.setWindowIcon( QIcon( ":/images/appicon.png" ) );

	CProgramWindow program;

	if( program.init() )
	{
		code = app.exec();

		program.shutdown();
	}

	return code;
}

