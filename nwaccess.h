#ifndef NWACCESS_H
#define NWACCESS_H

#include "emunwaccessclient.h"
#include "qtimer.h"
#include <QObject>

struct NWAEmulatorInfos {
    QString     name;
    QString     version;
    QString     id;
    QStringList commands;
};

struct MemoryDomain {
    QString         name;
    QString         access;
    unsigned int    size;
};

struct NWAGameInfos {
    QString name;
    QString file;
};

class NWAccess : public QObject
{
    Q_OBJECT
public:
    explicit NWAccess(QObject *parent = nullptr);
    void                        connectToHost();
    void                        requestGameInfos();
    void                        requestMemoriesDomains();
    const NWAGameInfos&         gameInfos() const;
    const NWAEmulatorInfos&     emulatorInfos() const;
    void                        readMemory(QList<unsigned int> addresses);
    void                        getMemories(QList<QPair<int, int> >);
    const QByteArray&           memoryDatas() const;
    const QList<MemoryDomain>&  memoriesDomains() const;
    bool                        hasMessage() const;
    void                        message(QString);

signals:
    void    connected();
    void    disconnected();
    void    ready();
    void    emulatorInfosDone();
    void    gameInfosDone();
    void    memoriesDomainsDone();
    void    getMemoriesDone();

private:
    EmuNWAccessClient*  client;
    NWAEmulatorInfos    m_emulatorInfos;
    NWAGameInfos        m_gameInfos;
    QList<MemoryDomain> m_memoriesDomains;
    QByteArray          m_memoryData;

    QTimer  checkConnected;
    void    onReadyRead();
};


#endif // NWACCESS_H
