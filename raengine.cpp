#include "sqapplication.h"
#include "raengine.h"

Q_LOGGING_CATEGORY(log_raEngine, "RAEngine")
#define sDebug() qCDebug(log_raEngine)
#define sInfo() qCInfo(log_raEngine)

RAEngine::RAEngine() {
    achievementChecker = new AchievementChecker(this);
    raWebAPIManager = new RAWebApiManager(this);
    usb2snes = new USB2snes();
    achievementChecker->allocRAM(0x20000);
    m_badgeImageProvider = new BadgeImageProvider;
    m_achievementsModel = new AchievementModel;
    setUsb2Snes();
    m_logged = false;
    setStatus(Status::None);
    setRAWebApiManager();
    m_hardcoreMode = false;
    setConnectionStatus(ConnectionStatus::NotConnected);
    pingTimer.setInterval(30000);
    connect(&pingTimer, &QTimer::timeout, this, [=] {
        raWebAPIManager->ping("I have no idea what going on", m_hardcoreMode);
    }
    );
    skipBadges = false;
    connect(achievementChecker, &AchievementChecker::achievementCompleted, this, &RAEngine::achievementCompleted);
    setRememberLogin(sqApp->settings()->value("login/RememberLogin").toBool());
    setHardcoreMode(sqApp->settings()->value("general/hardcore").toBool());
    if (m_rememberLogin)
    {
        if (sqApp->settings()->contains("login/Username") == false)
            return;
        QString userName = sqApp->settings()->value("login/Username").toString();
        QString userToken = sqApp->settings()->value("login/Usertoken").toString();
        QTimer::singleShot(0, this, [=] {
            raWebAPIManager->tokenLogin(userName, userToken);
        });
    }
}


bool RAEngine::login(QString username, QString password)
{
    sInfo() << "Remember me :" << m_rememberLogin;
    raWebAPIManager->regularLogin(username, password);
    return false;
}

Achievement RAEngine::achievement(unsigned int id)
{
    sDebug() << "QML requesting achievement " << id;
    return *m_achievements[id];
}

RAEngine::ConnectionStatus RAEngine::connectionStatus() const
{
    return m_connectionStatus;
}

void RAEngine::setConnectionStatus(ConnectionStatus newConnectionStatus)
{
    if (m_connectionStatus == newConnectionStatus)
        return;
    m_connectionStatus = newConnectionStatus;
    emit connectionStatusChanged();
}

AchievementModel *RAEngine::achievementsModel() const
{
    return m_achievementsModel;
}

void RAEngine::setStatus(Status status)
{
    sInfo() << "Status changed " << status;
    m_status = status;
    emit statusChanged();
}

void RAEngine::checkHardcoreCompatible()
{
    if (usb2snesInfos.secondField == "SD2SNES")
    {

    }
}

void RAEngine::testAddAchievement(Achievement ach)
{
    Achievement* newAch = new Achievement();
    *newAch = ach;
    m_achievementsModel->addAchievement(newAch);
    m_achievements[ach.id] = newAch;
}

BadgeImageProvider *RAEngine::bagdgeImageProvider() const
{
    return m_badgeImageProvider;
}

void RAEngine::setUsb2Snes()
{
    usb2snesCheckConnectedTimer.setInterval(500);
    connect(&usb2snesCheckConnectedTimer, &QTimer::timeout, this, [=]{
        if (usb2snes->state() == USB2snes::None)
        {
            sDebug() << "Usb2snes: Attempting to connect";
            usb2snes->connect();
        } else {
            usb2snesCheckConnectedTimer.stop();
        }
    });
    connect(usb2snes, &USB2snes::stateChanged, this, [=] {
        auto state = usb2snes->state();
        if (state == USB2snes::Ready)
        {
            setConnectionStatus(ConnectionStatus::Connected);
            usb2snesCheckInfoTimer.start();
            //usb2snes->infos();
        }
    });
    connect(usb2snes, &USB2snes::infoReceived, this, [=](Usb2SnesInfo infos) {
        sDebug() << "Usb2snes Info received";
        usb2snesInfos = infos;
        if (infos.isMenu == false)
        {
            usb2snesCheckInfoTimer.stop();
            usb2snes->getFile(infos.romPlayed);
        } else {
            setConnectionStatus(ConnectionStatus::NoGame);
        }
    });
    usb2snesCheckInfoTimer.setInterval(500);
    connect(&usb2snesCheckInfoTimer, &QTimer::timeout, this, [=] {
        usb2snes->infos();
    });
    connect(usb2snes, &USB2snes::fileGet, this, [=] {
        QByteArray romData = usb2snes->getFileData();
        if (romData.size() & 512)
            romData = romData.mid(512);
        QString md5 = QCryptographicHash::hash(romData, QCryptographicHash::Md5).toHex();
        setConnectionStatus(ConnectionStatus::Ready);
        raWebAPIManager->getGameId(md5);
    });
    connect(usb2snes, &USB2snes::getAddressDataReceived, this, [=] {
        achievementChecker->checkAchievements(usb2snes->getAsyncAdressData());
        usb2snes->getAsyncAddress(*achievementChecker->memoriesToCheck());
    });
}

void RAEngine::setRAWebApiManager()
{
    connect(raWebAPIManager, &RAWebApiManager::loginSuccess, this, [=] {
        if (m_rememberLogin)
        {
            sqApp->settings()->setValue("login/Username", raWebAPIManager->userInfos.userName);
            sqApp->settings()->setValue("login/RememberLogin", m_rememberLogin);
            sqApp->settings()->setValue("login/Usertoken", raWebAPIManager->userInfos.authToken);
        }
        emit loginDone(true);
        setStatus(Status::WaitingForUsb2Snes);
        m_achievements.clear();
        usb2snes->connect();
        usb2snesCheckConnectedTimer.start();
    });
    connect(raWebAPIManager, &RAWebApiManager::loginFailed, this, [=] {
        emit loginDone(false);
    });

    connect(raWebAPIManager, &RAWebApiManager::gameInfosDone, this, [=] {
        sInfo() << "Got achievement list";
        for (const auto& ach : raWebAPIManager->gameInfos.rawAchievements)
        {
            Achievement* newAchievement = new Achievement;
            newAchievement->id = ach.id;
            newAchievement->title = ach.title;
            newAchievement->description = ach.description;
            newAchievement->author = ach.author;
            newAchievement->badgeId = QFileInfo(QUrl(ach.badgeUrl).fileName()).baseName();
            newAchievement->badgeLockedId = QFileInfo(QUrl(ach.badgeLockedUrl).fileName()).baseName();
            newAchievement->points = ach.points;
            newAchievement->rarity = ach.rarity;
            newAchievement->rarityHardcore = ach.rarityHardcore;
            newAchievement->unlocked = false;
            newAchievement->hardcoreUnlocked = false;
            newAchievement->official = ach.flags == 3;
            badgesToDownload.append(ach.badgeUrl);
            badgesToDownload.append(ach.badgeLockedUrl);
            m_achievements[ach.id] = newAchievement;
        }
        raWebAPIManager->getUnlocks(false);
    });
    connect(raWebAPIManager, &RAWebApiManager::unlocksGotten, this, [=](bool hardcore) {
        if (hardcore == false)
        {
            for (unsigned int id : raWebAPIManager->unlocks)
                m_achievements[id]->unlocked = true;
            raWebAPIManager->getUnlocks(true);
        } else {
            for (unsigned int id : raWebAPIManager->unlocks)
                m_achievements[id]->hardcoreUnlocked = true;
            if (skipBadges)
            {
                startSession();
            } else {
                m_badgeImageProvider->getBadges(badgesToDownload);
            }
        }
    });
    connect(m_badgeImageProvider, &BadgeImageProvider::achievementBadgesReady, this, [=]{
          startSession();
    });
    connect(raWebAPIManager, &RAWebApiManager::startSessionDone, this, [=] {
        for (unsigned int id : raWebAPIManager->startSessionDatas.keys())
        {
            m_achievements[id]->unlockedTime = raWebAPIManager->startSessionDatas[id];
            m_achievementsModel->achievementUpdated(id);
        }
        emit sessionStarted();
        setStatus(Status::SessionStarted);
    });
    connect(raWebAPIManager, &RAWebApiManager::gameIdGotten, this, [=](int gameId) {
        setStatus(Status::GettingAchievementInfo);
        raWebAPIManager->getGameInfos(gameId);
    });
}

void RAEngine::achievementCompleted(unsigned int id)
{
    // We probably want to validate stuff when the server accept it?
    sInfo() << m_achievements[id]->title << " Achieved";
    if (m_hardcoreMode)
        m_achievements[id]->hardcoreUnlocked = true;
    else
        m_achievements[id]->unlocked = true;
    m_achievements[id]->unlockedTime = QDateTime::currentDateTime();
    m_achievementsModel->achievementUpdated(id);
    if (m_achievements[id]->official)
        raWebAPIManager->awardAchievement(id, m_hardcoreMode);
    emit achievementAchieved(*m_achievements[id]);
}

void RAEngine::startSession()
{
    achievementChecker->prepareCheck(raWebAPIManager->gameInfos.rawAchievements);
    sDebug() << "Setting before starting the session";
    m_achievementsModel->clear();
    for (unsigned int id : m_achievements.keys())
    {
        m_achievementsModel->addAchievement(m_achievements[id]);
    }
    sDebug() << "Done, starting the session";
    raWebAPIManager->startSession(m_hardcoreMode);
    pingTimer.start();
    usb2snes->getAsyncAddress(*achievementChecker->memoriesToCheck());
}

RAEngine::Status RAEngine::status() const
{
    return m_status;
}

bool RAEngine::hardcoreMode() const
{
    return m_hardcoreMode;
}

void RAEngine::setHardcoreMode(bool newHardcoreMode)
{
    if (m_hardcoreMode == newHardcoreMode)
        return;
    sqApp->settings()->setValue("general/hardcore", newHardcoreMode);
    m_hardcoreMode = newHardcoreMode;
    emit hardcoreModeChanged();
}

bool RAEngine::rememberLogin() const
{
    return m_rememberLogin;
}

void RAEngine::setRememberLogin(bool newRememberLogin)
{
    if (m_rememberLogin == newRememberLogin)
        return;
    m_rememberLogin = newRememberLogin;
    emit rememberLoginChanged();
}
