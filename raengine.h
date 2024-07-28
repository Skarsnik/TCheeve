#ifndef RAENGINE_H
#define RAENGINE_H

#include <QtQmlIntegration>
#include <QObject>
#include "sharedstruct.h"
#include "achievementmodel.h"

class RAEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum class ConnectionStatus {
        NotConnected,
        Connected,
        NoGame,
        Ready
    };
    Q_ENUM(ConnectionStatus)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus WRITE setconnectionStatus NOTIFY connectionStatusChanged FINAL)
    Q_PROPERTY(AchievementModel* achievementModel READ achievementModel WRITE setAchievementModel NOTIFY achievementModelChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    RAEngine();

    Q_INVOKABLE bool            login(QString username, QString password);
    Q_INVOKABLE Achievement*    achievement(unsigned int id) const;

    ConnectionStatus connectionStatus() const;
    void setconnectionStatus(ConnectionStatus newConnectionStatus);

    AchievementModel *achievementModel() const;
    void setAchievementModel(AchievementModel *newAchievementModel);

    void    testAddAchievement(Achievement ach);

    QString name() const;
    void setName(const QString &newName);

signals:
    void    connectionStatusChanged();
    void    achievementUnlocked(unsigned int);
    void    ready();

    void achievementModelChanged();

    void nameChanged();

private:
    QMap<unsigned int, Achievement*>    m_achievements;
    ConnectionStatus                    m_connectionStatus;
    AchievementModel*                   m_achievementModel = nullptr;
    QString m_name;
};

#endif // RAENGINE_H
