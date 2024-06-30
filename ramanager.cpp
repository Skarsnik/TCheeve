#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include "ramanager.h"
#include <QDebug>

const QString baseRequestUrl = "https://retroachievements.org/dorequest.php";

RAManager::RAManager() : QObject()
{
    connect(&networkManager, &QNetworkAccessManager::finished, this, &RAManager::networkReplyFinished);
}


void RAManager::networkReplyFinished(QNetworkReply* reply)
{
    qDebug() << reply->rawHeaderList();
    if (reply->error() != QNetworkReply::NoError)
        qDebug() << reply->errorString();
    if (m_currentQuerry == "login")
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit loginFailed();
        }
        const QByteArray data = reply->readAll();
        qDebug() << data;
        const QJsonObject jObj = QJsonDocument::fromJson(data).object();
        if (jObj.value("Success").toBool() == false)
        {
            emit loginFailed();
            return ;
        }
        userInfos.displayName = jObj.value("User").toString();
        userInfos.authToken = jObj.value("Token").toString();
        userInfos.hardcoreScore = jObj.value("Score").toInt();
        userInfos.softcoreScore = jObj.value("SoftcoreScore").toInt();
        emit loginSuccess();
        return ;
    }
    const QByteArray data = reply->readAll();
    //qDebug() << data;
    const QJsonObject jObj = QJsonDocument::fromJson(data).object();
    if (jObj.value("Success").toBool() == false)
    {
        emit requestFailed();
        return ;
    }
    if (m_currentQuerry == "gameid")
    {
        emit gameIdGotten(jObj.value("GameID").toInt());
        return ;
    }
    if (m_currentQuerry == "patch")
    {
        QJsonObject jObjPD = jObj.value("PatchData").toObject();
        gameInfos.title = jObjPD.value("Title").toString();
        gameInfos.imageIcon = jObjPD.value("ImageIcon").toString();
        gameInfos.imageIconUrl = jObjPD.value("ImageIconUrl").toString();
        QJsonArray jAchs = jObjPD.value("Achievements").toArray();
        for (const auto& jAch : jAchs)
        {
            Achievement a;
            QJsonObject jAchObj = jAch.toObject();
            a.author = jAchObj.value("Author").toString();
            a.title = jAchObj.value("Title").toString();
            a.id = jAchObj.value("ID").toInt();
            a.description = jAchObj.value("Description").toString();
            a.badgeName = jAchObj.value("BadgeName").toString();
            a.points = jAchObj.value("Points").toInt();
            a.category = jAchObj.value("Category").toString();
            a.type = jAchObj.value("Type").toInt(0);
            a.memAddrString = jAchObj.value("MemAddr").toString();
            gameInfos.achievements.append(a);
        }
        emit gameInfosDone();
        return ;
    }
}




void RAManager::regularLogin(const QString user, const QString password)
{
    userInfos.userName = user;
    doPostRequest("login", QMap<QString, QString>({{"u", user}, {"p", password}}));
}

void RAManager::getGameId(const QString md5hash)
{
    doPostRequest("gameid", QMap<QString, QString>({{"m", md5hash}}));
}

void RAManager::getGameInfos(const int gameId)
{
    doPostRequest("patch", QMap<QString, QString>({{"u", userInfos.userName}, {"t", userInfos.authToken}, {"g", QString::number(gameId)}}));
}

void RAManager::doPostRequest(QString function, QMap<QString, QString> keys)
{
    QNetworkRequest request;
    request.setUrl(baseRequestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, "rcheevos/0.1.0");
    QUrlQuery querry;
    querry.addQueryItem("r", function);
    for (const QString& key : keys.keys())
    {
        qDebug() << key << " : " << keys[key];
        querry.addQueryItem(key, QUrl::toPercentEncoding(keys[key]));
    }
    QByteArray content = querry.query().toLocal8Bit();
    m_currentQuerry = function;
    qDebug() << request.url() << " - " << content;
    networkManager.post(request, content);
}

