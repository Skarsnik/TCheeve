#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include "ramanager.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_RAManager, "RAManager")
#define sDebug() qCDebug(log_RAManager)
#define sInfo() qCInfo(log_RAManager)

const QString baseRequestUrl = "https://retroachievements.org/dorequest.php";


RAManager::RAManager() : QObject()
{
    connect(&networkManager, &QNetworkAccessManager::finished, this, &RAManager::networkReplyFinished);
    getImage = false;
}


void RAManager::networkReplyFinished(QNetworkReply* reply)
{
    sInfo() << reply->rawHeaderList();
    if (reply->error() != QNetworkReply::NoError)
        sInfo() << reply->errorString();
    if (getImage)
    {
        imageData = reply->readAll();
        emit achievementImageGotten();
        return ;
    }
    if (m_currentQuerry == "login")
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit loginFailed();
        }
        const QByteArray data = reply->readAll();
        sInfo() << data;
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
        sInfo() << data;
        emit gameIdGotten(jObj.value("GameID").toInt());
        return ;
    }
    if (m_currentQuerry == "patch")
    {
        QJsonObject jObjPD = jObj.value("PatchData").toObject();
        gameInfos.title = jObjPD.value("Title").toString();
        gameInfos.imageIcon = jObjPD.value("ImageIcon").toString();
        gameInfos.imageIconUrl = jObjPD.value("ImageIconUrl").toString();
        gameInfos.rawAchievements.clear();
        QJsonArray jAchs = jObjPD.value("Achievements").toArray();
        for (const auto& jAch : jAchs)
        {
            RawAchievement a;
            QJsonObject jAchObj = jAch.toObject();
            a.author = jAchObj.value("Author").toString();
            a.title = jAchObj.value("Title").toString();
            a.id = jAchObj.value("ID").toInt();
            a.description = jAchObj.value("Description").toString();
            a.badgeName = jAchObj.value("BadgeName").toString();
            a.points = jAchObj.value("Points").toInt();
            a.category = jAchObj.value("Category").toString();
            a.type = jAchObj.value("Type").toInt(0);
            a.badgeUrl = jAchObj.value("BadgeURL").toString();
            a.badgeLockedUrl = jAchObj.value("BadgeLockedURL").toString();
            a.badgeName = jAchObj.value("BadgeName").toString();
            a.memAddrString = jAchObj.value("MemAddr").toString();
            gameInfos.rawAchievements.append(a);
        }
        emit gameInfosDone();
        return ;
    }
    /*
     *    {"Success":true,"Unlocks":[{"ID":947,"When":1721336194}],"ServerNow":1721642272}
    */
    if (m_currentQuerry == "startsession")
    {
        startSessionDatas.clear();
        auto array = jObj.value("Unlocks").toArray();
        for (unsigned int i = 0; i < array.size(); i++)
        {
            const QJsonObject oUnlock = array[i].toObject();
            startSessionDatas[oUnlock.value("ID").toInt()] = QDateTime::QDateTime::fromSecsSinceEpoch(oUnlock.value("ID").toInteger());
        }
        emit startSessionDone();
        return ;
    }
    /*
      {"Success":true,"UserUnlocks":[],"GameID":337,"HardcoreMode":true}
      {"Success":true,"UserUnlocks":[947],"GameID":337,"HardcoreMode":false}
     **/
    if (m_currentQuerry == "unlocks")
    {
        unlocks.clear();
        auto array = jObj.value("UserUnlocks").toArray();
        for (unsigned int i = 0; i < array.size(); i++)
        {
            unlocks.append(array[i].toInt());
        }
        emit unlocksGotten();
        return ;
    }
}

void RAManager::startSession()
{
    doPostRequest("startsession", QMap<QString, QString>({
                                {"u", userInfos.userName},
                                {"t", userInfos.authToken},
                                {"g", QString::number(gameInfos.id)},
                                {"m", gameInfos.hash}}));
}

void RAManager::awardAchievement(unsigned int id, bool hardcore)
{
    doPostRequest("awardachievement", QMap<QString, QString>({
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"a", QString::number(id)},
                                    {"h", QString::number(hardcore)}}));
}

void RAManager::getUnlocks(bool hardcore)
{
    doPostRequest("unlocks", QMap<QString, QString>({
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"h", QString::number(hardcore)}}));
}

void RAManager::ping(QString message)
{
    doPostRequest("ping", QMap<QString, QString>({
                                    {"r", message},
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"g", QString::number(gameInfos.id)},
                                    {"m", gameInfos.hash},
                                    {"h", QString::number(1)}}));
}

void RAManager::regularLogin(const QString user, const QString password)
{
    userInfos.userName = user;
    doPostRequest("login", QMap<QString, QString>({{"u", user}, {"p", password}}));
}

void RAManager::getGameId(const QString md5hash)
{
    doPostRequest("gameid", QMap<QString, QString>({{"m", md5hash}}));
    gameInfos.hash = md5hash;
}

void RAManager::getGameInfos(const int gameId)
{
    doPostRequest("patch", QMap<QString, QString>({{"u", userInfos.userName}, {"t", userInfos.authToken}, {"g", QString::number(gameId)}}));
}

void RAManager::getAchievementImages(const QString url)
{
    networkManager.get(QNetworkRequest(url));
    getImage = true;
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
        sDebug() << key << " : " << keys[key];
        querry.addQueryItem(key, QUrl::toPercentEncoding(keys[key]));
    }
    QByteArray content = querry.query().toLocal8Bit();
    m_currentQuerry = function;
    sInfo() << request.url() << " - " << content;
    networkManager.post(request, content);
}

