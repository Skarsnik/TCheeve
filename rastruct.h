#ifndef RASTRUCT_H
#define RASTRUCT_H

#include <QString>
#include <QList>

struct RawAchievement;
struct Achievement;
#include <QDateTime>

struct UserInfos {
    QString userName;
    QString displayName;
    QString authToken;
    int     hardcoreScore;
    int     softcoreScore;
};

struct GameInfos {
    unsigned int id;
    QString hash;
    QString title;
    QString imageIcon;
    QString imageIconUrl;
    QList<RawAchievement> rawAchievements;
    //QList<Achievement> achievements;
};


/*
 *
 * {"ID":959,"MemAddr":"0xH00f341=1","Title":"Boomerang","Description":"Acquire the Boomerang","Points":5,
 * "Author":"HenrySwanson","Modified":1368727937,"Created":1368727926,"BadgeName":"01183",
 * "Flags":5,"Type":null,"Rarity":0,"RarityHardcore":0,"BadgeURL":"https:\/\/media.retroachievements.org\/Badge\/01183.png",
 * "BadgeLockedURL":"https:\/\/media.retroachievements.org\/Badge\/01183_lock.png"}
 */

// This is the class built from what the server send you

struct RawAchievement {
    unsigned int    id;
    QString         title;
    QString         description;
    QString         author;
    unsigned int    points;
    QString         badgeName;
    QString         category;
    QDateTime       createdDate;
    QString         badgeUrl;
    QString         badgeLockedUrl;
    QDateTime       createdTime;
    QDateTime       modifiedTime;
    unsigned int    rarity;
    unsigned int    rarityHardcore;
    bool            unlocked;
    unsigned int    flags; //TODO
    unsigned int    type;
    QString         memAddrString;
};


#endif // RASTRUCT_H
