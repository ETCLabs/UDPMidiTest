// Copyright (c) 2015 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QUdpSocket>
#include <QFile>
#include "windows.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnStart_pressed();
    void readData();
    void kbNoteOn(int note);
    void kbNoteOff(int note);
    void on_cbMidiOut_currentIndexChanged(int index);
    void on_leMSCData_textChanged(const QString &text);
    void updateMscCommand();
    void on_btnMSCSend_pressed();
    void on_cbLogToFile_pressed();
private:
    void rotateLogFile();
    void updateLogFileDisplay();
    Ui::MainWindow *ui;
    QUdpSocket *m_txSocket;
    QUdpSocket *m_rxSocket;
    HMIDIOUT m_midiOut;
    void midiMessageSend(quint8* msg, int length);
    void midiMessageRecieve(const QByteArray &data);
    QByteArray m_mscCommand;
    QByteArray m_mscData;
    QString m_logFileName;
    QFile *m_logFile = Q_NULLPTR;
    int m_msgCounter = 0;
};


#endif // MAINWINDOW_H
