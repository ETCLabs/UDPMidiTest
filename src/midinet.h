#ifndef MIDINET_H
#define MIDINET_H

#include <QObject>

class MidiNet : public QObject
{
    Q_OBJECT
public:
    explicit MidiNet(QObject *parent = nullptr);

signals:

public slots:
};

#endif // MIDINET_H