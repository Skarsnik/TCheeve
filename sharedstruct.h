#ifndef SHAREDSTRUCT_H
#define SHAREDSTRUCT_H

#include <QDateTime>
#include <QString>

/*This is the class to expose to QML
 *
 * */

struct Achievement {
    Q_GADGET
public:
    Q_PROPERTY(unsigned int achId MEMBER id)
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString description MEMBER description)
    Q_PROPERTY(QString author MEMBER author)
    Q_PROPERTY(unsigned int rarity MEMBER rarity)
    Q_PROPERTY(unsigned int rarityHardcore MEMBER rarityHardcore)
    Q_PROPERTY(unsigned int points MEMBER points)
    Q_PROPERTY(bool unlocked MEMBER unlocked)
    Q_PROPERTY(bool hardcoreUnlocked MEMBER hardcoreUnlocked)
    Q_PROPERTY(QDateTime unlockedTime MEMBER unlockedTime)
    Q_PROPERTY(QString badgeId MEMBER badgeId)
    Q_PROPERTY(QString badgeLockedId MEMBER badgeLockedId)
    Q_PROPERTY(bool official MEMBER official)

    unsigned int    id;
    QString         title;
    QString         description;
    QString         author;
    unsigned int    rarity;
    unsigned int    rarityHardcore;
    unsigned int    points;
    bool            unlocked;
    bool            hardcoreUnlocked;
    QDateTime       unlockedTime;
    QString         badgeId;
    QString         badgeLockedId;
    bool            official;
};


#endif // SHAREDSTRUCT_H
