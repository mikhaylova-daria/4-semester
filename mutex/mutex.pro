TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11
LIBS += -lboost_thread
SOURCES += test.cpp

HEADERS += \
    mutex.h
