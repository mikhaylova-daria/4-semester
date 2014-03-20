TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++11
LIBS += -lboost_thread
SOURCES += main.cpp

HEADERS += \
    tests.h \
    thread_pool.h

