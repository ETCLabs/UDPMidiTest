# Copyright (c) 2015 Electronic Theatre Controls, Inc., http://www.etcconnect.com
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

PRODUCT_VERSION = 1.0.1

QT       += core gui network xml svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpMidiTest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
    vkey/keylabel.cpp \
    vkey/pianokey.cpp \
    vkey/pianokeybd.cpp \
    vkey/pianoscene.cpp

HEADERS += \
        src/mainwindow.h \
    src/mididata.h \
    vkey/keyboardmap.h \
    vkey/keylabel.h \
    vkey/pianodefs.h \
    vkey/pianokey.h \
    vkey/pianokeybd.h \
    vkey/pianoscene.h

FORMS += \
        src/mainwindow.ui

LIBS += -lwinmm

RESOURCES += vkey/pianokeybd.qrc

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    DEPLOY_COMMAND = windeployqt
    DEPLOY_DIR = $$system_path($${_PRO_FILE_PWD_}/install/deploy)
    DEPLOY_TARGET = $$system_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT})
    DEPLOY_OPT = --dir $${DEPLOY_DIR}
    DEPLOY_CLEANUP = $$QMAKE_COPY $${DEPLOY_TARGET} $${DEPLOY_DIR}
    DEPLOY_INSTALLER = makensis /DPRODUCT_VERSION="$$PRODUCT_VERSION" $$system_path($${_PRO_FILE_PWD_}/install/win/install.nsi)
}

CONFIG( release , debug | release) {
    QMAKE_POST_LINK += $${PRE_DEPLOY_COMMAND} $$escape_expand(\\n\\t)
    QMAKE_POST_LINK += $${DEPLOY_COMMAND} $${DEPLOY_TARGET} $${DEPLOY_OPT}
    QMAKE_POST_LINK += $$escape_expand(\\n\\t) $${DEPLOY_CLEANUP}
    QMAKE_POST_LINK += $$escape_expand(\\n\\t) $${DEPLOY_INSTALLER}
}

