// EmuNWAccessClient-qt is a simple async qt implementation of EmuNWAccess Protocoll
// Copyright 2021 black-sliver
// Released under MIT license

#include "emunwaccessclient.h"
#include <QElapsedTimer>
#include <QtEndian>
#include <QtGlobal>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_emunwaccessclient, "EmuNWAccessClient", QtCriticalMsg)
#define cDebug() qCDebug(log_emunwaccessclient)

EmuNWAccessClient::EmuNWAccessClient(QObject *parent) : QObject(parent)
{
    _connected = false;
    _connecting = false;

    _socket = new QTcpSocket(this);
    _socket->connect(_socket, &QAbstractSocket::connected, this, &EmuNWAccessClient::on_socket_connected);
    _socket->connect(_socket, &QAbstractSocket::disconnected, this, &EmuNWAccessClient::on_socket_disconnected);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    _socket->connect(_socket, &QAbstractSocket::errorOccurred, this, &EmuNWAccessClient::on_socket_errorOccured);
#else
    _socket->connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &EmuNWAccessClient::on_socket_errorOccured);
#endif
    _socket->connect(_socket, &QAbstractSocket::readyRead, this, &EmuNWAccessClient::on_socket_readyRead);
}

QString EmuNWAccessClient::error() const
{
    return _lastError;
}
bool EmuNWAccessClient::isConnected() const
{
    return _connected;
}

bool EmuNWAccessClient::connectToHost(const QString& host, quint16 port)
{
    _lastError = "";
    _connected = false;
    _connecting = true;
    _socket->connectToHost(host, port);
    return _connecting || _connected;
}
bool EmuNWAccessClient::disconnectFromHost()
{
    _socket->disconnectFromHost();
    return true;
}
bool EmuNWAccessClient::waitForConnected(int msecs)
{
    // NOTE: connected fires first, so we can simply do this
    return _socket->waitForConnected(msecs);
}
bool EmuNWAccessClient::waitForDisconnected(int msecs)
{
    return _socket->waitForDisconnected(msecs);
}
bool EmuNWAccessClient::waitForBytesWritten(int msecs)
{
    return _socket->waitForBytesWritten(msecs);
}
bool EmuNWAccessClient::waitForReadyRead(int msecs)
{
    if (msecs<0) {
        while(_queue.empty()) _socket->waitForReadyRead(msecs);
    } else {
        QElapsedTimer t; t.start();
        int tmp = msecs;
        while (_queue.empty() && tmp>0) {
            _socket->waitForReadyRead(tmp);
            tmp = msecs-t.elapsed();
        }
    }
    return !_queue.empty();
}
EmuNWAccessClient::Reply EmuNWAccessClient::readReply()
{
    if (_queue.empty()) {
        return Reply::makeInvalid();
    } else {
        return _queue.dequeue();
    }
}
void EmuNWAccessClient::cmd(const QString &cmd, const QString &args)
{
    cDebug() << "Adding command to queue : " << cmd << args;
    _sent.enqueue(cmd);
    QByteArray buf = cmd.toUtf8();
    if (!args.isEmpty()) {
        buf += " ";
        buf += args.toUtf8();
    }
    buf += "\n";
    _socket->write(buf);
}
void EmuNWAccessClient::bcmd(const QString &cmd, const QString &args, const QByteArray &data)
{
    bcmdPrepare(cmd, args, data.length());
    bcmdData(data);
}

void EmuNWAccessClient::bcmdPrepare(const QString &cmd, const QString &args, const int datalength)
{

    cDebug() << "Adding command to queue : " << cmd << args;
    _sent.enqueue(cmd);
    QByteArray buf = "b"+cmd.toUtf8();
    if (!args.isEmpty()) {
        buf += " ";
        buf += args.toUtf8();
    }
    buf += "\n";
    _socket->write(buf);
    uint8_t binhdr[] = {0,0,0,0,0};
    qToBigEndian((uint32_t)datalength, binhdr+1);
    _socket->write((const char*)binhdr,sizeof(binhdr));
}

void EmuNWAccessClient::bcmdData(const QByteArray &data)
{
    _socket->write(data);
}

QString toStringPreferHex(int n)
{
    if (n<10) return QString::number(n, 10);
#ifdef NWACCESS_PREFER_SHORT // for numbers<100000 decimal representation may be shorter than hex
    if (n>=16 && n<100000) return QString::number(n, 10));
#endif
    return "$"+QString::number(n, 16);
}
QString toStringPreferDec(int n)
{
#ifdef NWACCESS_PREFER_SHORT // for numbers>999999 hex representation may be shorter than decimal
    if (n>999999) return "$" + QString::number(n, 16);
#endif
    return QString::number(n, 10);
}

void EmuNWAccessClient::cmdCoreWriteMemory(const QString& memory, const QByteArray& data, int start)
{
    if (start<1) cmdCoreWriteMemory(memory, data, "");
    else cmdCoreWriteMemory(memory, data, toStringPreferHex(start));
}
void EmuNWAccessClient::cmdCoreWriteMemory(const QString& memory, const QByteArray& data, QList< QPair<int,int> >& regions)
{
    // handle special cases / optional arguments
    if (regions.length() == 1 && (regions[0].second<0 || regions[0].second==data.length())) {
        cmdCoreWriteMemory(memory, data, regions[0].first);
        return;
    }
    QString s;
    for (auto& region : regions) {
        if (!s.isEmpty()) s += ";";
        s += toStringPreferHex(region.first) + ";" + toStringPreferDec(region.second);
    }
    cmdCoreWriteMemory(memory, data, s);
}
void EmuNWAccessClient::cmdCoreWriteMemory(const QString& memory, const QList< QPair<int,QByteArray> >& regions)
{
    QByteArray data;
    QList< QPair<int,int> > iregions;
    for (auto& region: regions) {
        iregions.push_back( { region.first, region.second.length() } );
        data += region.second;
    }
    cmdCoreWriteMemory(memory, data, iregions);
}
void EmuNWAccessClient::cmdCoreReadMemory(const QString& memory, int start, int len)
{
    if (start<1 && len<0) cmdCoreReadMemory(memory, "");

    QString s = toStringPreferHex(start);
    if (len>=0) s += ";" + toStringPreferDec(len);
    cmdCoreReadMemory(memory, s);
}
void EmuNWAccessClient::cmdCoreReadMemory(const QString& memory, const QList< QPair<int,int> >& regions)
{
    // handle special cases / optional arguments
    if (regions.length() == 1 && regions[0].second<0) {
        cmdCoreReadMemory(memory, regions[0].first);
        return;
    }
    QString s;
    for (const auto& region : regions) {
        if (!s.isEmpty()) s += ";";
        s += toStringPreferHex(region.first) + ";" + toStringPreferDec(region.second);
    }
    //qDebug() << "Core memoris arg : " << s;
    cmdCoreReadMemory(memory, s);
}

void EmuNWAccessClient::cmdCoreWriteMemoryPrepare(const QString& memory, QList< QPair<int,int> > regions)
{
    QString s;
    int len = 0;
    for (auto& region : regions) {
        if (!s.isEmpty()) s += ";";
        s += toStringPreferHex(region.first) + ";" + toStringPreferDec(region.second);
        if (region.second<0) throw;
        len += region.second;
    }
    cmdCoreWriteMemoryPrepare(memory, s, len);
}
void EmuNWAccessClient::cmdCoreWriteMemoryData(const QByteArray& data)
{
    bcmdData(data);
}

void EmuNWAccessClient::on_socket_connected()
{
    _connecting = false;
    _connected = true;
    emit connected();
}
void EmuNWAccessClient::on_socket_disconnected()
{
    _connecting = false;
    _connected = false;
    _buffer.clear();
    _sent.clear();
    emit disconnected();
}
void EmuNWAccessClient::on_socket_errorOccured(QAbstractSocket::SocketError)
{
    if (_connecting) {
        _socket->disconnectFromHost();
        _connecting = false;
        _connected = false;
        _lastError = _socket->errorString();
        emit connectError();
    }
    else {
        cDebug() << "Socket Error:" << _socket->errorString();
        _socket->disconnectFromHost();
        _socket->waitForDisconnected(1);
        _connecting = false;
        _connected = false;
        _lastError = _socket->errorString();
        emit disconnected();
    }
}
void EmuNWAccessClient::on_socket_readyRead()
{
    _buffer += _socket->readAll();
    while (!_buffer.isEmpty()) {
        if (_sent.empty()) {
            cDebug() << "Reply without command";
            _lastError = "Protocol Error";
            _socket->disconnectFromHost();
            break;
        } else if (_buffer[0] == '\0') {
            cDebug() << "Received binary reply";
            if (_buffer.length()<5) break; // need more data
            // try to parse binary reply
            quint32 len = qFromBigEndian<quint32>(_buffer.constData()+1);
            if (len>1*1024*1024*1024) {
                cDebug() << "Received Binary >1GB";
                _lastError = "Protocol Error";
                _socket->disconnectFromHost();
            }
            if ((unsigned)_buffer.length()-5<len) break; // need more data
            _queue.enqueue(Reply::makeBinary(_buffer.mid(5, len), _sent.dequeue()));
            _buffer = _buffer.mid(5+len);
            emit readyRead();
        } else if (_buffer[0] == '\n') {
            // try to parse ascii reply
            int p = _buffer.indexOf("\n\n");
            if (p<0) {
                if (_buffer.length()>1*1024*1024) {
                    cDebug() << "Received Ascii >1MB";
                    _lastError = "Protocol Error";
                    _socket->disconnectFromHost();
                }
                break; // need more data
            }
            cDebug() << "Received ascii reply";
            _queue.enqueue(Reply::makeAscii(QString::fromUtf8(_buffer.left(p+2)), _sent.dequeue()));
            _buffer = _buffer.mid(p+2);
            emit readyRead();
        } else {
            cDebug() << "Protocol Error";
            _lastError = "Protocol Error";
            _socket->disconnectFromHost();
            break;
        }
    }
}

QDebug operator<<(QDebug debug, const EmuNWAccessClient::Reply& reply)
{
    if (!reply.isValid) {
        return debug << "No reply";
    } else if (reply.isError) {
        return debug << "Error:" << reply.error << "for" << reply.cmd;
    } else if (reply.isBinary) {
        return debug << "Binary:" << reply.binary << "for" << reply.cmd;
    } else {
        return debug << "Ascii:" << reply.ascii << "for" << reply.cmd;
    }
}
QMap<QString,QString> EmuNWAccessClient::Reply::toMap() const
{
    QMap<QString,QString> map;
    for (const auto& s : ascii) {
        int p = s.indexOf(":");
        if (p<0)
            map[s]="";
        else
            map[s.left(p)] = s.mid(p+1);
    }
    return map;
}
QList< QMap<QString,QString> > EmuNWAccessClient::Reply::toMapList() const
{
    QList< QMap<QString,QString> > lst;
    QMap<QString,QString> map;
    for (const auto& s : ascii) {
        int p = s.indexOf(":");
        QString key = (p<0) ? s : s.left(p);
        QString val = (p<0) ? "" : s.mid(p+1);
        if (map.contains(key)) {
            lst.push_back(map);
            map.clear();
        }
        map[key] = val;
    }
    if (!map.isEmpty()) lst.push_back(map);
    return lst;
}
EmuNWAccessClient::Reply EmuNWAccessClient::Reply::makeBinary(const QByteArray& binary, QString&& cmd)
{
    Reply reply;
    reply.cmd = cmd;
    reply.binary = binary;
    reply.parse();
    return reply;
}
EmuNWAccessClient::Reply EmuNWAccessClient::Reply::makeBinary(QByteArray&& binary, QString&& cmd)
{
    Reply reply;
    reply.cmd = cmd;
    reply.binary = binary;
    reply.parse();
    return reply;
}
EmuNWAccessClient::Reply EmuNWAccessClient::Reply::makeAscii(const QString& text, QString&& cmd)
{
    Reply reply;
    reply.cmd = cmd;
    reply.ascii = text.split("\n");
    reply.parse();
    return reply;
}
EmuNWAccessClient::Reply EmuNWAccessClient::Reply::makeAscii(QString&& text, QString&& cmd)
{
    Reply reply;
    reply.cmd = cmd;
    reply.ascii = text.split("\n");
    reply.parse();
    return reply;
}
EmuNWAccessClient::Reply EmuNWAccessClient::Reply::makeInvalid()
{
    return Reply();
}
void EmuNWAccessClient::Reply::parse()
{
    isBinary = ascii.empty();
    isValid = isBinary || (ascii.size()>=3 && ascii.front().isEmpty() && (ascii.rbegin()++)->isEmpty() && ascii.back().isEmpty());
    isAscii = isValid && !isBinary;
    if (!isBinary) {
        if (isValid) {
            ascii.pop_back();
            ascii.pop_back();
            ascii.pop_front();
        }
        for (auto& s: ascii) {
            if (s.startsWith("error:")) {
                isError = true;
                error = s.mid(6);
                break;
            }
        }
    }
}
const QString& EmuNWAccessClient::Reply::operator[](const QString& key)
{
    if (_mapCache.empty()) {
        _mapCache = toMap();
    }
    return _mapCache[key];
}
bool EmuNWAccessClient::Reply::contains(const QString& key)
{
    if (_mapCache.empty()) {
        _mapCache = toMap();
    }
    return _mapCache.contains(key);
}
