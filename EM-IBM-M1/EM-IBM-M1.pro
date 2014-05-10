TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


QMAKE_CXXFLAGS += \-std=c++11
            \-o3
LIBS += \-lboost_thread
        \-o3

SOURCES += main.cpp

HEADERS += \
    corpus_reader.h \
    EM-IBM_2.h

