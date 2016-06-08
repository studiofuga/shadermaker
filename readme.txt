

 Shader Maker - a simple, cross-platform GLSL editor
====================================================

This document tells you how to compile the source code, generate the HTML
documentation and run the application.

To install this software, simply unpack it into a directory of your choice.
This software neither registers itself on the system nor creates any config
files.  So you can simply delete it if you want to uninstall it.


Compilation:
------------

Prerequisite for compiling the source code: Qt 4.2 or higher, including
headers and libraries.
Under Mac OS X: you need also the WebServices package, which is not
installed by default with all the other developer stuff.
(Another library, GLee, which is used by this software, is already included
in the source code directory.) 

Generate a project file / Makefile:
    Windoze:      qmake -tp vc ShaderMaker.pro
    Linux / Mac:  qmake -unix ShaderMaker.pro
    Mac OS X:     qmake ShaderMaker.pro

Then compile using Visual Studio / make / XCode.

If you have a clean Mac, *first* install Xcode, *then* Qt.

In order to package a self-contained app bundle for Mac's, use Qt's macdeployqt (in /usr/bin).



Documentation:
--------------

If you want to build the documentation, you need Doxygen 1.5.4 or higher.
Run doxygen like this:

$ cd /path/to/glsleditor/doc/
$ doxygen

Afterwards, the HTML documentation can be found in doc/html/index.html .
The doc contains a user manual for "Shader Maker" and the source code documentation.

The user's manual is organized as a set of short and nice tutorials
(see the links on the entry page index.html).


Known issues:
-------------

- On Mac OS X (10.5), the options 'Switch to SDI view' and 'Switch to MDI view' freeze the program.
  The reason is unknown. Because the feature works well on Windows and Linux, we believe it to be
  a bug in Qt. If you encounter this problem, you can hard-code your
  preferred view by editing the method CEditor::init() in editor.cpp.

- On Mac OS and Windows XP, there is a problem with button colors.
  Several buttons in the GUI bring up a color selection dialog.
  The selected color is then used as the button's background color.
  Unfortunately, this does not work on some platforms. On Windows XP,
  this problem appears only when using the default theme. When switching to the 'classic'
  theme, the buttons are drawn correctly. The behaviour on Windows Vista is unknown and not tested.
  The reason is unknown, but seems to be a bug in Qt or in the underlying operating system.
  A possible workaround whould be to use a built-in style of Qt, where Qt draws everything by itself.
  For aesthetic reasons, however, we opted not to use that.

