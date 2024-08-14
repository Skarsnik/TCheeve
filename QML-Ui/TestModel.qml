
import QtQuick 2.15
import QtQuick.Controls 2.15

// This is just data to fill when there is
// no engine

/*
    unsigned int    achId;
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
  */


ListModel {
    id: testModel
    ListElement {
        achId : 100
        title : "My first achievement"
        description : "Opening RAStuff"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : true
        hardcoreUnlocked : false
        unlockedTime : "2024-06-12 20:00"
        badgeId : 's45'
        badgeLockedId : "100_lock"
    }
    ListElement {
        achId : 102
        title : "Why not a second"
        description : "Making a second achievement"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 15:00"
        badgeId : "102"
        badgeLockedId : "102_lock"
    }
    ListElement {
        achId : 587
        title : "Me three"
        description : "I like random achievement, this is the third"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 20:00"
        badgeId : "587"
        badgeLockedId : "587_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
    ListElement {
        achId : 899
        title : "Another one"
        description : "kdlsklds, this is the fourth"
        author : "Skarsnik"
        rarity : 0
        rarityHardcore : 0
        points : 4
        unlocked : false
        hardcoreUnlocked : false
        unlockedTime : "2024-06-25 21:00"
        badgeId : "899"
        badgeLockedId : "899_lock"
    }
}
