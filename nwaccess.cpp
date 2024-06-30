#include <QDebug>
#include "nwaccess.h"

NWAccess::NWAccess(QObject *parent)
    : QObject{parent}
{
    client = new EmuNWAccessClient();
    connect(client, &EmuNWAccessClient::readyRead, this, &NWAccess::onReadyRead);
    connect(client, &EmuNWAccessClient::connected, this, [=] {
        client->cmdEmulatorInfo();
        emit connected();
    });
    connect(client, &EmuNWAccessClient::disconnected, this, &NWAccess::disconnected);
    connect(client, &EmuNWAccessClient::disconnected, this, [=] {
        connectToHost();
    });
    connect(client, &EmuNWAccessClient::connectError, this, [=] {
        qDebug() << client->error();
        connectToHost();
    });
    checkConnected.setInterval(500);
}

void NWAccess::connectToHost()
{
    client->connectToHost("localhost", 0xBEEF);
    qDebug() << "Trying to connect";
    //checkConnected.start();
}

void NWAccess::requestGameInfos()
{
    m_gameInfos.name = "";
    m_gameInfos.file = "";
    client->cmdEmulationStatus();
}

void NWAccess::requestMemoriesDomains()
{
    client->cmdCoreMemories();
}

const NWAGameInfos &NWAccess::gameInfos() const
{
    return m_gameInfos;
}

const NWAEmulatorInfos &NWAccess::emulatorInfos() const
{
    return m_emulatorInfos;
}

void NWAccess::getMemories(QList<QPair<int, int> >mems)
{
    qDebug() << mems;
    client->cmdCoreReadMemory("WRAM", mems);
}

const QByteArray &NWAccess::memoryDatas() const
{
    return m_memoryData;
}

const QList<MemoryDomain>& NWAccess::memoriesDomains() const
{
    return m_memoriesDomains;
}

void NWAccess::onReadyRead()
{
    EmuNWAccessClient::Reply reply = client->readReply();
    qDebug() << "Reply" << reply.cmd << reply.toMap();
    if (reply.cmd == "EMULATOR_INFO")
    {
        auto map = reply.toMap();
        m_emulatorInfos.name = map["name"];
        m_emulatorInfos.version = map["version"];
        m_emulatorInfos.id = map["id"];
        m_emulatorInfos.commands = map["commands"].split(",");
        emit emulatorInfosDone();
        emit ready();
        return;
    }
    if (reply.cmd == "EMULATION_STATUS")
    {
        auto map = reply.toMap();
        if (map["status"] == "no_game" || map["status"] == "stopped")
        {
            emit gameInfosDone();
        } else {
            client->cmdGameInfo();
        }
        return;
    }
    if (reply.cmd == "GAME_INFO")
    {
        auto map = reply.toMap();
        m_gameInfos.name = map["name"];
        m_gameInfos.file = map["file"];
        emit gameInfosDone();
        return;
    }
    if (reply.cmd == "CORE_READ")
    {
        m_memoryData = reply.binary;
        emit getMemoriesDone();
        return;
    }
    if (reply.cmd == "CORE_MEMORIES")
    {
        auto map = reply.toMapList();
        m_memoriesDomains.clear();
        for (const auto& a : map)
        {
            MemoryDomain d;
            d.access = a["access"];
            d.name = a["name"];
            d.size = a["size"].toUInt();
            m_memoriesDomains.append(d);
        }
        emit memoriesDomainsDone();
        return ;
    }
}


bool NWAccess::hasMessage() const
{
    return m_emulatorInfos.commands.contains("MESSAGE");
}

void NWAccess::message(QString msg)
{
    client->cmd("MESSAGE", msg);
}
