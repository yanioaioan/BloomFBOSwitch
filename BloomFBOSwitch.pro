TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += bloom.cpp

include(deployment.pri)
qtcAddDeployment()
CONFIG+=c++11

INCLUDEPATH += ../common/include
#OTHER_FILES += shaders/*.glsl
LIBS += -lGL -lGLEW -lGLU
LIBS += -lXi -ldl -lX11 -lXrandr -lXinerama -lXcursor -lXxf86vm -lglfw3 -lSOIL -L/home/yioannidis/BloomFBOSwitch/external/SOIL/build
OTHER_FILES += *.vs \
                *.frag

INCLUDEPATH += . ./includes ./external/SOIL/src
#/usr/include $$NGLPATH/include ../../../common/include $$IGLPATH/include  $$EIGENPATH $$NOISEINCPATH
