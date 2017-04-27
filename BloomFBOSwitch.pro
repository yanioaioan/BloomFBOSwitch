TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt
QT+=gui opengl core


SOURCES += ssao.cpp

include(deployment.pri)
qtcAddDeployment()
CONFIG+=c++11

NGLPATH=$$(NGLDIR)


INCLUDEPATH += ../common/include
INCLUDEPATH += $$NGLPATH/include
#OTHER_FILES += shaders/*.glsl
LIBS += -lGL -lGLEW -lGLU -lNGL -L$$NGLPATH/lib
LIBS += -lXi -ldl -lX11 -lXrandr -lXinerama -lXcursor -lXxf86vm -lglfw3 -lSOIL -L/home/yioannidis/BloomFBOSwitch/external/SOIL/build
LIBS+= -lassimp
OTHER_FILES += $$PWD/ssaoShaders/*.vs \
               $$PWD/ssaoShaders/*.frag

INCLUDEPATH += . ./includes ./external/SOIL/src
#/usr/include $$NGLPATH/include ../../../common/include $$IGLPATH/include  $$EIGENPATH $$NOISEINCPATH

DISTFILES += \
    testpingpong2.vs \
    testpingpong2.fs


