#include <QMetaProperty>
#include <QLoggingCategory>
#include "achievementmodel.h"


Q_LOGGING_CATEGORY(log_achievementModel, "AchievementModel")
#define sDebug() qCDebug(log_achievementModel)
#define sInfo() qCInfo(log_achievementModel)


AchievementModel::AchievementModel(QObject *parent)
    : QAbstractListModel{parent}
{}

int AchievementModel::rowCount(const QModelIndex &parent) const
{
    return m_achievements.size();
}

QVariant AchievementModel::data(const QModelIndex &index, int role) const
{
    return Achievement::staticMetaObject.property(role - Qt::UserRole).readOnGadget(m_achievements.at(index.row()));
}

QHash<int, QByteArray> AchievementModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    qDebug() << Achievement::staticMetaObject.propertyCount();
    for (unsigned int i = 0; i < Achievement::staticMetaObject.propertyCount(); i++)
    {
        roles[Qt::UserRole + i] = Achievement::staticMetaObject.property(i).name();
    }
    sDebug() << "Roles " << roles;
    return roles;
}

void AchievementModel::addAchievement(Achievement *ach)
{
    m_achievements.append(ach);
}

void AchievementModel::achievementUpdated(unsigned int id)
{
    unsigned int i = 0;
    for (const auto& ach : m_achievements)
    {
        if (ach->id == id)
        {
            break;
        }
        i++;
    }
    emit dataChanged(index(i), index(i));
}

void AchievementModel::clear()
{
    m_achievements.clear();
    endResetModel();
}

void AchievementModel::endInsert()
{
    endResetModel();
}
