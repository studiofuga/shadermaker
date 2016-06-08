###############################################################################
#
# ShaderMaker.pro - Shader Maker's main project file
#
# Use this file to generate a Makefile or project file, resp.
#
#    Windoze:      qmake -tp vc ShaderMaker.pro
#    Linux / Mac:  qmake -unix ShaderMaker.pro
#    Mac OS X:     qmake -spec macx-xcode ShaderMaker.pro
#


###############################################################################
# main configuration
###############################################################################

CONFIG += qt release warn_on opengl
QT += opengl

TEMPLATE = app
TARGET = ShaderMaker
DESTDIR = ..

OBJECTS_DIR = obj
MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR

DEPENDPATH += $$OBJECTS_DIR
INCLUDEPATH += .

# Mac
QMAKE_INFO_PLIST = CustomInfo.plist
# qmake will copy this file to ShaderMaker.app/Contents/Info.plist

#
# WIN32:
#
win32 {
	CONFIG += debug_and_release
}

macx:LIBS     += -lz -framework Carbon

###############################################################################
#	RESOURCES
###############################################################################

RESOURCES = images/images.qrc


###############################################################################
# sources
###############################################################################

SOURCES += editor.cpp \
           editwindow.cpp \
           geometry.cpp \
           glwidget.cpp \
           highlighter.cpp \
           lightwidget.cpp \
           main.cpp \
           objmodel.cpp \
           programwindow.cpp \
           scene.cpp \
           scenewidget.cpp \
           shader.cpp \
           sourceeditor.cpp \
           texturewidget.cpp \
           uniform.cpp \
           uniformwidget.cpp \
           universalslider.cpp \
           vertexstream.cpp \
           glee/GLee.c


###############################################################################
# headers
###############################################################################

HEADERS += application.h \
           camera.h \
           config.h \
           editor.h \
           editwindow.h \
           glwidget.h \
           light.h \
           lightwidget.h \
           model.h \
           programwindow.h \
           scene.h \
           scenewidget.h \
           shader.h \
           sourceeditor.h \
           stdshader.h \
           texture.h \
           texturewidget.h \
           uniform.h \
           uniformwidget.h \
           universalslider.h \
           vector.h \
           vertexstream.h \
           glee/GLee.h

