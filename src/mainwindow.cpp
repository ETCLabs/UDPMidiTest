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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mididata.h"
#include <QMessageBox>
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QMetaEnum>
#include <QDebug>
#include <QFileDialog>
#include <QDate>
#include <QDateTime>
#include <QAbstractEventDispatcher>
#include <QTimer>

QByteArray eosEncode(float value)
{
    /* There are four simple rules for formatting:
    1. Specify the cue number first, and then the cue list
    2. Place a "3" in front of every digit of the number
    3. Place a "2E" wherever there is a decimal
    4. Place a "00" when separating a cue number from the cue list */

    QString asAscii = QString::number(value);
    QByteArray result;
    for(int i=0; i<asAscii.length(); i++)
    {
        if(asAscii[i]==QChar('.'))
            result.append((char)0x2e);
        else
            result.append((char)0x30 | asAscii[i].digitValue());
    }
    return result;
}

QString stringToHex(quint8 value)
{
    QString result = QString("%1").arg(value, 2, 16, QChar('0'));
    return result.toUpper();
}


static int msecsTo(const QTime & at) {
  const int msecsPerDay = 24 * 60 * 60 * 1000;
  int msecs = QTime::currentTime().msecsTo(at);
  if (msecs < 0) msecs += msecsPerDay;
  return msecs;
}




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_txSocket = Q_NULLPTR;
    m_rxSocket = Q_NULLPTR;

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface i, interfaces)
    {
        QList<QNetworkAddressEntry> entries = i.addressEntries();
        QString result;
        foreach(QNetworkAddressEntry e, entries)
        {
            if(!result.isEmpty()) result.append(QString(","));
            result.append(e.ip().toString());
        }
        result.append(" - ");
        result.append(i.humanReadableName());
        ui->cbNic->addItem(result,QVariant(i.hardwareAddress()));
        ui->keyboardWidget->setShowLabels(true);
    }


    ui->cbMidiOut->addItem(tr("Don't Play Locally"));
    UINT nDevs = midiOutGetNumDevs();
    for(UINT i=0; i<nDevs; i++)
    {
        MIDIOUTCAPS capabilities;
        midiOutGetDevCaps(i,
           &capabilities,
           sizeof(MIDIOUTCAPS)
        );
        ui->cbMidiOut->addItem( QString::fromWCharArray(capabilities.szPname) );
    }

    connect(ui->keyboardWidget, SIGNAL(noteOn(int)), this, SLOT(kbNoteOn(int)));
    connect(ui->keyboardWidget, SIGNAL(noteOff(int)), this, SLOT(kbNoteOff(int)));

    QMetaEnum midiCommandFormats = QMetaEnum::fromType<MidiData::MidiCommandFormats>();
    for(int i=0; i<midiCommandFormats.keyCount(); i++)
    {
        int value = midiCommandFormats.value(i);
        const char *key = midiCommandFormats.key(i);
        QString hex = QString("%1").arg(value, 2, 16, QChar('0'));
        hex = hex.toUpper();
        QString desc = QString("%1 (%2)").arg(key).arg(hex);
        ui->cbMSCCommandFormat->addItem(desc, QVariant(value));
    }


    QMetaEnum midiCommands = QMetaEnum::fromType<MidiData::MidiMscCommands>();
    for(int i=0; i<midiCommands.keyCount(); i++)
    {
        int value = midiCommands.value(i);
        const char *key = midiCommands.key(i);
        QString hex = QString("%1").arg(value, 2, 16, QChar('0'));
        hex = hex.toUpper();
        QString desc = QString("%1 (%2)").arg(key).arg(hex);
        ui->cbMSCCommand->addItem(desc, QVariant(value));
    }

    connect(ui->sbMSCDevId, SIGNAL(valueChanged(int)), this, SLOT(updateMscCommand()));
    connect(ui->cbMSCCommand, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMscCommand()));
    connect(ui->cbMSCCommandFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(updateMscCommand()));
    connect(ui->leMSCData, SIGNAL(textChanged(QString)), this, SLOT(updateMscCommand()));

    updateLogFileDisplay();

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnStart_pressed()
{
    QNetworkInterface selected;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface i, interfaces)
    {
       if(i.hardwareAddress()==ui->cbNic->currentData().toString())
       {
           selected = i;
       }
    }

    m_txSocket = new QUdpSocket(this);
    m_rxSocket = new QUdpSocket(this);

    // Bind TX socket
    bool ok = false;

    // Bind to first IPv4 address on selected NIC
    QHostAddress localHostAddress;

    foreach (QNetworkAddressEntry ifaceAddr, selected.addressEntries())
    {
        if (ifaceAddr.ip().protocol() == QAbstractSocket::IPv4Protocol)
        {
            localHostAddress = ifaceAddr.ip();
            ok = m_txSocket->bind(ifaceAddr.ip());
            m_txSocket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, QVariant(1));
            m_txSocket->setMulticastInterface(selected);
            if(ok) qDebug() << "TX Socket : Bound to IP:" << ifaceAddr.ip().toString();
            break;
        }
    }

    // Bind RX socket
    ok = false;

    // Bind to mutlicast address
    ok = m_rxSocket->bind(localHostAddress,
                   ui->sbTargetPort->value(),
                   QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);

    if(!ok) qDebug() << "Error binding RX socket";

    // Join multicast on selected NIC
    /*if (ok)
    {
        m_rxSocket->setMulticastInterface(selected);
        ok |= m_rxSocket->joinMulticastGroup(QHostAddress(MIDI_MULTICAST_ADDR), selected);
    } */

    if(!ok) qDebug() << "Error joining RX socket to mcast group";

    connect(m_rxSocket, SIGNAL(readyRead()), this, SLOT(readData()));

    ui->btnStart->setEnabled(false);
    ui->sbTargetPort->setEnabled(false);
    ui->leTargetIp->setEnabled(false);
    ui->cbNic->setEnabled(false);

}


void MainWindow::readData()
{
    while (m_rxSocket->hasPendingDatagrams())
    {
        m_msgCounter++;
        QNetworkDatagram datagram = m_rxSocket->receiveDatagram();

        if(ui->cbLogAllInput->isChecked())
        {
            ui->lvRxMessages->addItem(QString("%1:%2 - %3")
                                  .arg(datagram.senderAddress().toString())
                                  .arg(datagram.senderPort())
                                  .arg(QString(datagram.data()))
                                  );
        }
        if(m_logFile)
        {
            QString logMsg = QString("%1,%2,%3\r\n")
                    .arg(QTime::currentTime().toString("hh:mm:ss:zzz"))
                    .arg(datagram.senderAddress().toString())
                    .arg(QString::fromLatin1(datagram.data()));
            m_logFile->write(logMsg.toUtf8());
            m_logFile->flush();
            updateLogFileDisplay();
        }


        midiMessageRecieve(datagram.data());
    }

}

void MainWindow::on_cbMidiOut_currentIndexChanged(int index)
{
    midiOutClose(m_midiOut);
    midiOutOpen(&m_midiOut, index-1, NULL, NULL, CALLBACK_NULL );
}

void MainWindow::kbNoteOn(int note)
{
    quint8 midiMsg[3];
    midiMsg[0] = MidiData::MIDI_NOTE_ON;
    midiMsg[1] = note;
    midiMsg[2] = 64; // velocity

    midiMessageSend(midiMsg, 3);
}

void MainWindow::kbNoteOff(int note)
{
    quint8 midiMsg[3];
    midiMsg[0] = MidiData::MIDI_NOTE_OFF;
    midiMsg[1] = note;
    midiMsg[2] = 64; // velocity

    midiMessageSend(midiMsg, 3);
}

void MainWindow::midiMessageSend(quint8 *msg, int length)
{
    if(length==3)
    {
        if(ui->cbMidiOut->currentIndex()!=0 && ui->cbPlayTx->isChecked())
        {
            quint32 packedMsg;
            packedMsg = msg[0];
            packedMsg |= msg[1] << 8;
            packedMsg |= msg[2] << 16;
            midiOutShortMsg(m_midiOut, packedMsg);
        }
    }

    QString message("MIDI");
    for(int i=0; i<length; i++)
        message.append(QString(" %1").arg(msg[i],2, 16, QChar('0')).toUpper());

    ui->teBuffer->appendPlainText(message);

    if(m_txSocket)
        m_txSocket->writeDatagram(message.toLatin1(), QHostAddress(ui->leTargetIp->text()), ui->sbTargetPort->value());
}

void MainWindow::midiMessageRecieve(const QByteArray &data)
{
    // Check it's MIDI
    if(!(data[0]=='M' && data[1]=='I' && data[2]=='D' && data[3]=='I'))
        return;

    QString s(data.mid(4));
    QStringList chunks = s.split(QChar(' '), QString::SkipEmptyParts);
    QByteArray theMidi;
    for(int i=0; i<chunks.count(); i++)
    {
        bool ok = false;
        quint8 value = chunks[i].toInt(&ok, 16);
        if(ok)
            theMidi.append(1, value);
    }

    if(theMidi.size()==3)
    {
        if(ui->cbMidiOut->currentIndex()!=0 && ui->cbPlayRx->isChecked())
        {
            quint32 packedMsg;
            packedMsg = (quint8)theMidi[0];
            packedMsg |= (quint8)theMidi[1] << 8;
            packedMsg |= (quint8)theMidi[2] << 16;
            midiOutShortMsg(m_midiOut, packedMsg);
        }
    }

    if(memcmp(theMidi.data(), &MidiData::TIMECODE_START, sizeof(MidiData::TIMECODE_START)) == 0)
    {
        int index = sizeof(MidiData::TIMECODE_START);
        quint8 hours = theMidi.at(index);
        index++;
        quint8 minutes = theMidi.at(index);
        index++;
        quint8 seconds = theMidi.at(index);
        index++;
        quint8 frames = theMidi.at(index);

        QString value = QString("%1:%2:%3:%4")
                .arg(hours, 2, 10, QLatin1Char('0'))
                .arg(minutes, 2, 10, QLatin1Char('0'))
                .arg(seconds, 2, 10, QLatin1Char('0'))
                .arg(frames, 2, 10, QLatin1Char('0'));

        ui->nTimecode->display(value);
    }

}


void MainWindow::updateMscCommand()
{
    m_mscCommand.clear();
    m_mscCommand.append((char)0xF0);
    m_mscCommand.append((char)0x7F);
    m_mscCommand.append((char)0xFF & ui->sbMSCDevId->value());
    m_mscCommand.append((char)0x02);
    m_mscCommand.append((char)0xFF & ui->cbMSCCommandFormat->currentData().toInt());
    m_mscCommand.append((char)0xFF & ui->cbMSCCommand->currentData().toInt());
    m_mscCommand.append(m_mscData);
    m_mscCommand.append((char)0xF7);

    QString composedCommand;
    for(int i=0; i<m_mscCommand.length(); i++)
    {
        composedCommand.append(stringToHex(m_mscCommand.at(i)));
        composedCommand.append(" ");
    }

    ui->leMSCComposedData->setText(composedCommand);
}

void MainWindow::on_leMSCData_textChanged(const QString &text)
{
    m_mscData.clear();
    if(ui->cbMscDataType->currentIndex() == 0)
    {
        // Data should be Hex bytes
        QString dataString = text;
        dataString.replace(QString(" "), QString());
        if(dataString.length() % 2)
        {
            // Not properly formatted - invalid
            ui->lbInvalidData->setVisible(true);
            ui->btnMSCSend->setEnabled(false);
            return;
        }
        for(int i=0; i<dataString.length(); i+=2)
        {
            QString byteString;
            byteString.resize(2);
            byteString[0] = dataString[i];
            byteString[1] = dataString[i+1];
            bool ok = false;
            quint8 value = byteString.toInt(&ok, 16);
            if(!ok)
            {
                ui->lbInvalidData->setVisible(true);
                ui->btnMSCSend->setEnabled(false);
                m_mscData.clear();
                return;
            }
            m_mscData.append((char) value);
        }
    }
    if(ui->cbMscDataType->currentIndex()==1)
    {
        // Eos cue format
        /* There are four simple rules for formatting:
        1. Specify the cue number first, and then the cue list
        2. Place a "3" in front of every digit of the number
        3. Place a "2E" wherever there is a decimal
        4. Place a "00" when separating a cue number from the cue list */

        // Entered format is Cuelist/Cue
        // Or cuelist/
        // Or cue
        QRegExp regex("([0-9.]*)/?([0-9.]+)?");
        QList<float> parts;
        bool noCueNum = text.endsWith(QChar('/'));


        int pos = regex.indexIn(text);
        if(pos>-1)
        {
            for(int i=0; i<regex.captureCount(); i++)
            {
                QString capture = regex.cap(i+1);
                bool ok;
                float value = capture.toFloat(&ok);
                if(!ok)
                    parts << 0;
                else
                    parts << value;
            }
        }

        if(parts.count()<1 || parts.count()>2 || (parts[0]==0 && parts[1]==0))
        {
            ui->lbInvalidData->setVisible(true);
            ui->btnMSCSend->setEnabled(false);
            m_mscData.clear();
            return;
        }

        // Cuelist/cue
        if(parts[0]>0 && parts[1]>0)
        {
            m_mscData.append(eosEncode(parts[1]));
            m_mscData.append((char)0x00);
            m_mscData.append(eosEncode(parts[0]));
        }

        // Cuelist/
        if(noCueNum)
        {
            m_mscData.append((char)0x00);
            m_mscData.append(eosEncode(parts[0]));
        }
        else
        {
            m_mscData.append(eosEncode(parts[0]));
            m_mscData.append((char)0x00);
        }



    }


    updateMscCommand();
    ui->lbInvalidData->setVisible(false);
    ui->btnMSCSend->setEnabled(true);
}

void MainWindow::on_btnMSCSend_pressed()
{
    midiMessageSend((quint8*)m_mscCommand.data(), m_mscCommand.length());
}

void MainWindow::on_cbLogToFile_pressed()
{
    m_msgCounter = 0;
    if(!ui->cbLogToFile->isChecked())
    {
        m_logFileName = QFileDialog::getSaveFileName(this, tr("Save Log File"), QString(), tr("Log Files (*.log)"));
        if(m_logFileName.isEmpty())
        {
            m_logFileName = "";
            m_logFile = Q_NULLPTR;
            ui->cbLogToFile->setChecked(false);
            return;
        }
        m_logFileName.chop(3); // Remove the .log postfix
        rotateLogFile();
        ui->cbLogToFile->setChecked(true);


        // Rotate the log at midnight
        auto timer = new QTimer(QAbstractEventDispatcher::instance());
        timer->start(msecsTo(QTime(0,0,0)));
        QObject::connect(timer, &QTimer::timeout, [=, &timer]{
          this->rotateLogFile();
          timer->deleteLater();
        });
    }
    else {
        ui->lbLogInfo->clear();
    }
}

void MainWindow::rotateLogFile()
{
    if(m_logFile)
    {
        m_logFile->close();
        m_logFile = Q_NULLPTR;
    }

    QString fileName = m_logFileName + QString("%1.log").arg(QDate::currentDate().toString("yy_MM_dd"));

    m_logFile = new QFile(fileName);
    if(!m_logFile->open(QIODevice::WriteOnly))
    {
        ui->cbLogToFile->setChecked(false);
        m_logFile = Q_NULLPTR;
    }
    updateLogFileDisplay();

}

void MainWindow::updateLogFileDisplay()
{
    if(!m_logFile)
        ui->lbLogInfo->clear();
    else {
       QString logInfo = tr("Logging to %1 : %2 messages")
               .arg(m_logFile->fileName())
               .arg(m_msgCounter);
       ui->lbLogInfo->setText(logInfo);
    }
}
