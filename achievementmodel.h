#ifndef ACHIEVEMENTMODEL_H
#define ACHIEVEMENTMODEL_H

#include <QAbstractListModel>
#include "sharedstruct.h"

class AchievementModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AchievementModel(QObject *parent = nullptr);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray>  roleNames() const;
    void                    addAchievement(Achievement* ach);
    void                    achievementUpdated(unsigned int id);
    void                    clear();

private:
    QList<Achievement*> m_achievements;
};

#endif // ACHIEVEMENTMODEL_H
