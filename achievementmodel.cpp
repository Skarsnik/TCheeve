#include <QMetaProperty>
#include "achievementmodel.h"

AchievementModel::AchievementModel(QObject *parent)
    : QAbstractListModel{parent}
{}

int AchievementModel::rowCount(const QModelIndex &parent) const
{
    return m_achievements.size();
}

QVariant AchievementModel::data(const QModelIndex &index, int role) const
{
    return Achievement::staticMetaObject.property(Qt::UserRole - role).readOnGadget(m_achievements[index.row()]);
}

QHash<int, QByteArray> AchievementModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    qDebug() << Achievement::staticMetaObject.propertyCount();
    for (unsigned int i = 0; i < Achievement::staticMetaObject.propertyCount(); i++)
    {
        roles[Qt::UserRole + i] = Achievement::staticMetaObject.property(i).name();
    }
    return roles;
}

void AchievementModel::addAchievement(Achievement *ach)
{
    m_achievements.append(ach);
}
