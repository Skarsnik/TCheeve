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

signals:
    void    loginFailed();
    void    loginSuccess();
    void    gameIdGotten(int gameId);
    void    gameInfosDone();
    void    requestFailed();
    void    requestError();

private:
    QNetworkAccessManager networkManager;

    void    doPostRequest(QString function, QMap<QString, QString> keys);
    QString m_currentQuerry;

    void    networkReplyFinished(QNetworkReply* reply);
};

#endif // RAMANAGER_H
