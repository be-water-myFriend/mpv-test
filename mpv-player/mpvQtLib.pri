
SRC_DIR = $$PWD/
INCLUDEPATH += $$SRC_DIR

SOURCES += \
    $$SRC_DIR/MpvPlayer.cpp \
    $$SRC_DIR/mpvwidget.cpp

HEADERS += \
    $$SRC_DIR/MpvPlayer.h \
    $$SRC_DIR/mpvwidget.h \
    $$SRC_DIR/qthelper.hpp

#################################################################
INCLUDEPATH += $$PWD/mpv-dev-x86_64/include
LIBS += -L$$PWD/mpv-dev-x86_64/ -llibmpv
#################################################################


