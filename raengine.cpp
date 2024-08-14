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
    setConnectionStatus(ConnectionStatus::NotConnected);
    pingTimer.setInterval(30000);
    connect(&pingTimer, &QTimer::timeout, this, [=] {
        raWebAPIManager->ping("I have no idea what going on");
    }
    );
    skipBadges = false;
    connect(achievementChecker, &AchievementChecker::achievementCompleted, this, &RAEngine::achievementCompleted);
    login("Skarsnik", "gobbla42");
}


bool RAEngine::login(QString username, QString password)
{
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
        }
    });
    connect(usb2snes, &USB2snes::infoReceived, this, [=](Usb2SnesInfo infos) {
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
        emit loginDone(true);
        setStatus(Status::WaitingForUsb2Snes);
        usb2snes->connect();
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
                raWebAPIManager->getAchievementImages(badgesToDownload.first());
            }
        }
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
    connect(raWebAPIManager, &RAWebApiManager::achievementImageGotten, this, [=] {
        QString badgeId = QFileInfo(QUrl(badgesToDownload.first()).fileName()).baseName();
        sDebug() << badgeId << badgesToDownload.size();
        m_badgeImageProvider->addBadgePixmap(badgeId, raWebAPIManager->imageData);
        badgesToDownload.remove(0);
        if (badgesToDownload.isEmpty())
        {
            startSession();
        } else {
            raWebAPIManager->getAchievementImages(badgesToDownload.first());
        }
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
    m_achievements[id]->unlocked = true;
    m_achievements[id]->unlockedTime = QDateTime::currentDateTime();
    m_achievementsModel->achievementUpdated(id);
    raWebAPIManager->awardAchievement(id, false);
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
    m_achievementsModel->endInsert();
    sDebug() << "Done, starting the session";
    raWebAPIManager->startSession();
    pingTimer.start();
    usb2snes->getAsyncAddress(*achievementChecker->memoriesToCheck());
}

RAEngine::Status RAEngine::status() const
{
    return m_status;
}
