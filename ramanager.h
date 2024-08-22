#ifndef RAMANAGER_H
#define RAMANAGER_H
#include "rastruct.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RAWebApiManager : public QObject
{
    Q_OBJECT

public:
    RAWebApiManager(QObject *parent = nullptr);
    void    regularLogin(const QString user, const QString password);
    void    tokenLogin(const QString username, const QString token);
    void    getGameId(const QString md5hash);
    void    getGameInfos(const int gameId);
    UserInfos userInfos;
    GameInfos gameInfos;
    QByteArray imageData;
    QMap<unsigned int, QDateTime>   startSessionDatas;
    QList<unsigned int>             unlocks;

    void    getAchievementImages(const QString url);
    void    awardAchievement(unsigned int id, bool hardcore);
    void    getUnlocks(bool hardcore);
    void    ping(QString message, bool hardcore);

    void    startSession(bool hardcore);

signals:
    void    loginFailed();
    void    loginSuccess();
    void    gameIdGotten(int gameId);
    void    gameInfosDone();
    void    requestFailed();
    void    requestError();
    void    achievementImageGotten();
    void    awardAchievementSuccess(unsigned int id);
    void    startSessionDone();
    void    unlocksGotten(bool hardcore);

private:
    QNetworkAccessManager networkManager;

    void    doPostRequest(QString function, QMap<QString, QString> keys);
    bool    getImage;
    QString m_currentQuerry;
    QDateTime   startedRequetTime;

    void    networkReplyFinished(QNetworkReply* reply);
};

#endif // RAMANAGER_H
