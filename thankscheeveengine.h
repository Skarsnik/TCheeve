#ifndef THANKSCHEEVEENGINE_H
#define THANKSCHEEVEENGINE_H

#include <QtQmlIntegration>
#include <QObject>
#include <QTimer>
#include "ramanager.h"
#include "sharedstruct.h"
#include "achievementmodel.h"
#include "achievementchecker.h"
#include "usb2snes.h"
#include "badgeimageprovider.h"

class ThanksCheeveEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    enum class ConnectionStatus {
        NotConnected,
        Connected,
        NoGame,
        CheckingHardcoreCompatibility,
        GettingGame,
        Ready
    };
    enum class Status {
        None,
        WaitingForUsb2Snes,
        GettingAchievementInfo,
        SessionStarted
    };

    Q_ENUM(ConnectionStatus)
    Q_ENUM(Status)
    Q_PROPERTY(ConnectionStatus connectionStatus READ connectionStatus NOTIFY connectionStatusChanged FINAL)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
    Q_PROPERTY(AchievementModel* achievementsModel READ achievementsModel CONSTANT FINAL)
    Q_PROPERTY(bool rememberLogin READ rememberLogin WRITE setRememberLogin NOTIFY rememberLoginChanged FINAL)
    Q_PROPERTY(bool hardcoreMode READ hardcoreMode NOTIFY hardcoreModeChanged FINAL)
    ThanksCheeveEngine();

    Q_INVOKABLE bool            login(QString username, QString password);
    // Q_GADGET does not allow to be returned as pointer, maybe fallback to
    // returning QObject*
    Q_INVOKABLE Achievement     achievement(unsigned int id);

    ConnectionStatus            connectionStatus() const;
    void                        setConnectionStatus(ConnectionStatus newConnectionStatus);

    AchievementModel *achievementsModel() const;

    void    testAddAchievement(Achievement ach);
    BadgeImageProvider*    bagdgeImageProvider() const;

    ThanksCheeveEngine::Status status() const;

    bool hardcoreMode() const;

    bool rememberLogin() const;
    void setRememberLogin(bool newRememberLogin);

signals:
    void    connectionStatusChanged();
    void    statusChanged();
    void    achievementAchieved(Achievement);
    void    achievementPrimed(Achievement);
    void    achievementUnprimed(Achievement);
    void    ready();
    void    loginDone(bool success);
    void    sessionStarted();

    void hardcoreModeChanged();

    void rememberLoginChanged();

private:
    QMap<unsigned int, Achievement*>    m_achievements;
    ConnectionStatus                    m_connectionStatus;
    Status                              m_status;
    AchievementModel*                   m_achievementsModel = nullptr;
    BadgeImageProvider*                 m_badgeImageProvider;

    QTimer                              usb2snesCheckConnectedTimer;
    QTimer                              usb2snesCheckInfoTimer;
    USB2snes*                           usb2snes;
    Usb2SnesInfo                        usb2snesInfos;

    QTimer                              pingTimer;
    bool                                m_logged;
    RAWebApiManager*                    raWebAPIManager;
    AchievementChecker*                 achievementChecker;

    QList<QString>                      badgesToDownload;
    bool                                skipBadges;
    bool                                m_hardcoreMode;


    void    setAchievementModel(AchievementModel *newAchievementModel);
    void    setStatus(ThanksCheeveEngine::Status status);
    void    checkHardcoreCompatible();
    void    setHardcoreMode(bool newHardcoreMode);

    void    setUsb2Snes();
    void    setRAWebApiManager();
    void    gameIdGotten(int id);
    void    usb2snesGameStarted(QString romPlaying);
    void    achievementCompleted(unsigned int id);
    void    achievementIPrimed(unsigned int id);
    void    achievementIUnprimed(unsigned int id);
    void    getBadges();
    void    startSession();
    bool m_rememberLogin;
};

#endif // THANKSCHEEVEENGINE_H
