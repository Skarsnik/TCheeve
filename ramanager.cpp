#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include "ramanager.h"
#include <QDebug>
#include <QLoggingCategory>
#include "rc_version.h"

Q_LOGGING_CATEGORY(log_RAManager, "RAManager")
#define sDebug() qCDebug(log_RAManager)
#define sInfo() qCInfo(log_RAManager)

const QString baseRequestUrl = "https://retroachievements.org/dorequest.php";


RAWebApiManager::RAWebApiManager(QObject *parent)
    : QObject{parent}
{
    connect(&networkManager, &QNetworkAccessManager::finished, this, &RAWebApiManager::networkReplyFinished);
}


void RAWebApiManager::networkReplyFinished(QNetworkReply* reply)
{
    sDebug() << "Time for request" << startedRequetTime.msecsTo(QDateTime::currentDateTime());
    if (reply->operation() == QNetworkAccessManager::GetOperation)
        return ;
    sInfo() << reply->rawHeaderList();
    if (reply->error() != QNetworkReply::NoError)
        sInfo() << reply->errorString();

    const QByteArray data = reply->readAll();
    if (m_currentQuerry != "patch")
        sDebug() << data;
    const QJsonObject jObj = QJsonDocument::fromJson(data).object();
    if (m_currentQuerry != "login" && (reply->error() != QNetworkReply::NoError || jObj.value("Success").toBool() == false))
    {
        emit requestFailed();
        goto rfinished_end_and_delete;
    }

    if (m_currentQuerry == "login")
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit loginFailed();
            goto rfinished_end_and_delete;
        }
        if (jObj.value("Success").toBool() == false)
        {
            emit loginFailed();
            goto rfinished_end_and_delete;
        }
        userInfos.displayName = jObj.value("User").toString();
        userInfos.authToken = jObj.value("Token").toString();
        userInfos.hardcoreScore = jObj.value("Score").toInt();
        userInfos.softcoreScore = jObj.value("SoftcoreScore").toInt();
        emit loginSuccess();
        goto rfinished_end_and_delete;
    }
    if (m_currentQuerry == "gameid")
    {
        sInfo() << data;
        gameInfos.id = jObj.value("GameID").toInt();
        emit gameIdGotten(gameInfos.id);
        goto rfinished_end_and_delete;
    }
    /*
     * {"ID":959,"MemAddr":"0xH00f341=1","Title":"Boomerang","Description":"Acquire the Boomerang","Points":5,
    * "Author":"HenrySwanson","Modified":1368727937,"Created":1368727926,"BadgeName":"01183",
    * "Flags":5,"Type":null,"Rarity":0,"RarityHardcore":0,"BadgeURL":"https:\/\/media.retroachievements.org\/Badge\/01183.png",
    * "BadgeLockedURL":"https:\/\/media.retroachievements.org\/Badge\/01183_lock.png"}
     *
    */
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
            a.flags = jAchObj.value("Flags").toInt();
            a.unlocked = false;
            gameInfos.rawAchievements.append(a);
        }
        emit gameInfosDone();
        goto rfinished_end_and_delete;
    }
    /*
     *    {"Success":true,"Unlocks":[{"ID":947,"When":1721336194}],"ServerNow":1721642272}
    */
    if (m_currentQuerry == "startsession")
    {
        startSessionDatas.clear();
        QJsonArray array;
        if (jObj.contains("Unlocks"))
            array = jObj.value("Unlocks").toArray();
        if (jObj.contains("HardcoreUnlocks"))
            array = jObj.value("HardcoreUnlocks").toArray();
        for (unsigned int i = 0; i < array.size(); i++)
        {
            const QJsonObject oUnlock = array[i].toObject();
            startSessionDatas[oUnlock.value("ID").toInt()] = QDateTime::QDateTime::fromSecsSinceEpoch(oUnlock.value("WHEN").toInteger());
        }
        emit startSessionDone();
        goto rfinished_end_and_delete;
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
            for (auto& rawAch : gameInfos.rawAchievements)
            {
                if (rawAch.id == array[i].toInt())
                    rawAch.unlocked = true;
            }

        }
        emit unlocksGotten(jObj.value("HardcoreMode").toBool());
        goto rfinished_end_and_delete;
    }
    /*post URL : https://retroachievements.org/dorequest.php - Content : r=awardachievement&u=Skarsnik&t=88TrmuWJvnOCbw2g&a=944&h=0&m=608c22b8ff930c62dc2de54bcd6eba72&v=5e9193ea81b31899d1705a94f9cfb85c
    {"Success":true,"AchievementsRemaining":108,"Score":0,"SoftcoreScore":5,"AchievementID":944} */
    if (m_currentQuerry == "awardachievement")
    {
        emit awardAchievementSuccess(jObj.value("AchievementID").toInt());
        goto rfinished_end_and_delete;
    }
rfinished_end_and_delete:
    reply->deleteLater();
}

// r=startsession&u=Skarsnik&t=88TrmuWJvnOCbw2g&g=355&h=1&m=608c22b8ff930c62dc2de54bcd6eba72&l=11.0

void RAWebApiManager::startSession(bool hardcore)
{
    doPostRequest("startsession", QMap<QString, QString>({
                                {"u", userInfos.userName},
                                {"t", userInfos.authToken},
                                {"g", QString::number(gameInfos.id)},
                                {"m", gameInfos.hash},
                                {"h", QString::number(hardcore)},
                                {"l", RCHEEVOS_VERSION_STRING}}));
}

void RAWebApiManager::awardAchievement(unsigned int id, bool hardcore)
{
    /*
     *
    md5_init(&md5);
    snprintf(buffer, sizeof(buffer), "%u", api_params->achievement_id);
    md5_append(&md5, (md5_byte_t*)buffer, (int)strlen(buffer));
    md5_append(&md5, (md5_byte_t*)api_params->username, (int)strlen(api_params->username));
    snprintf(buffer, sizeof(buffer), "%d", api_params->hardcore ? 1 : 0);
    md5_append(&md5, (md5_byte_t*)buffer, (int)strlen(buffer));
    md5_finish(&md5, digest);
    rc_format_md5(buffer, digest);
    rc_url_builder_append_str_param(&builder, "v", buffer);

    https://retroachievements.org/dorequest.php - Content : r=awardachievement&u=Skarsnik&t=88TrmuWJvnOCbw2g&a=944&h=0&m=608c22b8ff930c62dc2de54bcd6eba72&v=5e9193ea81b31899d1705a94f9cfb85c
     */
    QCryptographicHash md5Hash(QCryptographicHash::Md5);
    md5Hash.addData(QString::number(id).toLocal8Bit());
    md5Hash.addData(userInfos.userName.toLocal8Bit());
    md5Hash.addData(QString::number(hardcore).toLocal8Bit());

    doPostRequest("awardachievement", QMap<QString, QString>({
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"a", QString::number(id)},
                                    {"h", QString::number(hardcore)},
                                    {"v", md5Hash.result().toHex()}}));
}

void RAWebApiManager::getUnlocks(bool hardcore)
{
    doPostRequest("unlocks", QMap<QString, QString>({
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"h", QString::number(hardcore)},
                                    {"g", QString::number(gameInfos.id)}}));
}

/*
 * post URL : https://retroachievements.org/dorequest.php - Content :
 * r=ping&u=Skarsnik&t=88TrmuWJvnOCbw2g&g=355&m=Exploring+the+Light+World+%e2%80%a2+3+%e2%9d%a4%ef%b8%8f+%e2%80%a2+no+bottles&h=0&x=608c22b8ff930c62dc2de54bcd6eba72
                {"Success":true}
*/
void RAWebApiManager::ping(QString message, bool hardcore)
{
    doPostRequest("ping", QMap<QString, QString>({
                                    {"m", message},
                                    {"u", userInfos.userName},
                                    {"t", userInfos.authToken},
                                    {"g", QString::number(gameInfos.id)},
                                    {"x", gameInfos.hash},
                                    {"h", QString::number(hardcore)}}));
}

void RAWebApiManager::regularLogin(const QString user, const QString password)
{
    userInfos.userName = user;
    doPostRequest("login", QMap<QString, QString>({{"u", user}, {"p", password}}));
}

void RAWebApiManager::tokenLogin(const QString username, const QString token)
{
    userInfos.userName = username;
    doPostRequest("login", QMap<QString, QString>({{"u", username}, {"t", token}}));
}

void RAWebApiManager::getGameId(const QString md5hash)
{
    doPostRequest("gameid", QMap<QString, QString>({{"m", md5hash}}));
    gameInfos.hash = md5hash;
}

void RAWebApiManager::getGameInfos(const int gameId)
{
    doPostRequest("patch", QMap<QString, QString>({{"u", userInfos.userName}, {"t", userInfos.authToken}, {"g", QString::number(gameId)}}));
}

void RAWebApiManager::doPostRequest(QString function, QMap<QString, QString> keys)
{
    startedRequetTime = QDateTime::currentDateTime();
    QNetworkRequest request;
    request.setUrl(baseRequestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::UserAgentHeader, "T.Cheeve/v0.1 (" + deviceName + ")");
    QUrlQuery querry;
    querry.addQueryItem("r", function);
    for (const QString& key : keys.keys())
    {
        //sDebug() << key << " : " << keys[key];
        querry.addQueryItem(key, QUrl::toPercentEncoding(keys[key]));
    }
    QByteArray content = querry.query(QUrl::EncodeUnicode | QUrl::EncodeSpaces).toLocal8Bit();
    // Cheevos use + for space, sigh
    content.replace("%20", "+");
    m_currentQuerry = function;
    QByteArray contentToLog = content;
    // To filter out real password from logs
    if (function == "login")
    {
        querry.removeQueryItem("p");
        querry.addQueryItem("p", "ThisIsNotAPasswordThisIsATribute");
        contentToLog = querry.query(QUrl::EncodeUnicode | QUrl::EncodeSpaces).toLocal8Bit();
        contentToLog.replace("%20", "+");
    }
    sInfo() << request.url() << " - " << contentToLog;
    networkManager.post(request, content);
}

