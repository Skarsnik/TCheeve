#include "raengine.h"

RAEngine::RAEngine() {
    m_name = "Il fait beau";
}

bool RAEngine::login(QString username, QString password)
{
    return false;
}

Achievement* RAEngine::achievement(unsigned int id) const
{
    return m_achievements[id];
}

RAEngine::ConnectionStatus RAEngine::connectionStatus() const
{
    return m_connectionStatus;
}

void RAEngine::setconnectionStatus(ConnectionStatus newConnectionStatus)
{
    if (m_connectionStatus == newConnectionStatus)
        return;
    m_connectionStatus = newConnectionStatus;
    emit connectionStatusChanged();
}

AchievementModel *RAEngine::achievementModel() const
{
    return m_achievementModel;
}

void RAEngine::setAchievementModel(AchievementModel *newAchievementModel)
{
    if (m_achievementModel == newAchievementModel)
        return;
    m_achievementModel = newAchievementModel;
    emit achievementModelChanged();
}

void RAEngine::testAddAchievement(Achievement ach)
{
    Achievement* newAch = new Achievement();
    *newAch = ach;
    m_achievementModel->addAchievement(newAch);
    m_achievements[ach.id] = newAch;
}

QString RAEngine::name() const
{
    return m_name;
}

void RAEngine::setName(const QString &newName)
{
    if (m_name == newName)
        return;
    m_name = newName;
    emit nameChanged();
}
