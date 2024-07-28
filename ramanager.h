#ifndef RAMANAGER_H
#define RAMANAGER_H
#include "rastruct.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RAManager : public QObject
{
    Q_OBJECT

public:
    RAManager();
    void    regularLogin(const QString user, const QString password);
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
    void    ping(QString message);

    void    startSession();

signals:
    void    loginFailed();
    void    loginSuccess();
    void    gameIdGotten(int gameId);
    void    gameInfosDone();
    void    requestFailed();
    void    requestError();
    void    achievementImageGotten();
    void    startSessionDone();
    void    unlocksGotten();

private:
    QNetworkAccessManager networkManager;

    void    doPostRequest(QString function, QMap<QString, QString> keys);
    bool    getImage;
    QString m_currentQuerry;

    void    networkReplyFinished(QNetworkReply* reply);
};

#endif // RAMANAGER_H
