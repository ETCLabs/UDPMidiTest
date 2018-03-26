#include "mididata.h"
#include <QDebug>

MidiDataTx::MidiDataTx()
{
    m_position = 0;
    m_sequence = 0;
    uuid = QUuid::createUuid();
}

void MidiDataTx::addMessage(QByteArray midiMessage)
{
    m_sequence++;
    m_messages.push(midiMessage);
    if(m_messages.length() > MIDI_RINGBUFFER_SIZE)
        m_messages.pop_front();
}

QByteArray MidiDataTx::getPackedData()
{
    QByteArray result;
    result.append(MIDI_HEADER, strlen(MIDI_HEADER));
    result.append(uuid.toRfc4122());
    result.append((char) m_sequence);
    foreach(QByteArray data, m_messages)
    {
        result.append((char)data.length());
        result.append(data);
    }

    return result;
}


/***************************************************************/

MidiDataRx::MidiDataRx()
{

}

void MidiDataRx::processDatagram(const QByteArray &data)
{
    int pos = 0;
    QByteArray header = data.mid(0, strlen(MIDI_HEADER));
    QString str = QString::fromUtf8(header);
    if(str != MIDI_HEADER)
    {
        qDebug() << "Invalid Packet";
        return;
    }
    pos += strlen(MIDI_HEADER);

    QByteArray cidBytes = data.mid(strlen(MIDI_HEADER), 16);
    QUuid cid = QUuid::fromRfc4122(cidBytes);

    pos += 16;

    bool newSource = false;

    int index = 0;

    if(!m_uuids.contains(cid))
    {
        qDebug() << "New source found : " << cid;
        m_uuids << cid;
        m_sequences << new quint8;
        newSource = true;
        index = m_uuids.count() - 1;
    }
    else
    {
        index = m_uuids.indexOf(cid);
    }

    quint8 sentSequence = data.at(pos);
    if(newSource)
    {
        // Take the first number from a source
        *m_sequences.at(index) = sentSequence;
    }

    quint8 lastSequence = *m_sequences.at(index);

    qDebug() << "Got sequence " << sentSequence;

    if(sentSequence==lastSequence)
    {
        // Nothing changed so ignore
        return;
    }

    if(sentSequence<lastSequence)
    {
        // Went backwards, drop it
        qDebug() << "Sequence went backward, ignoring";
        return;
    }
    if(sentSequence>lastSequence)
    {
        qDebug() << "Sequence went forward";

        // TODO: handle large jumps
        *m_sequences.at(index) = sentSequence;
    }

    pos += 1;

    QList<QByteArray> packetMessages;
    // Unpack messages
    while(pos<data.length())
    {
        quint8 msgLength = data.at(pos);
        pos++;

        QByteArray message = data.mid(pos, msgLength);
        packetMessages << message;
        pos += msgLength;
    }

    // Empty packet?
    if(packetMessages.count()==0)
        return;

    int numNewMsgs = sentSequence - lastSequence;

    // Append the last N messages to our list
    qDebug() << "Got " << numNewMsgs << " new messages";

    for(int i=packetMessages.count()-1; i>packetMessages.count()-numNewMsgs-1; i--)
    {
        midiMessages << packetMessages[i];
    }



}
