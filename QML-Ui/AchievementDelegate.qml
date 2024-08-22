/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts
import fr.nyo.RAEngine;


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


Rectangle {
    // This list all the ach properpy
    required property int achId
    required property string title // : "test"
    required property string description //: "default description"
    required property int rarity
    required property int rarityHardcore
    required property int points
    required property bool unlocked
    required property bool hardcoreUnlocked
    required property date unlockedTime
    required property string badgeId
    required property string badgeLockedId

    width : 400
    height : 80
    color: {
        var munlocked = false;
        if (MainEngine.hardcoreMode)
            munlocked = hardcoreUnlocked;
        else
            munlocked = unlocked;
        return munlocked ? "yellow" : "white"
    }
    GridLayout {
        columns : 2
        rows : 2
        anchors.fill: parent
        Image {
            Layout.row: 0
            Layout.column: 0
            Layout.rowSpan: 2
            Layout.fillHeight: true
            fillMode: Image.PreserveAspectFit
            id: achievementImage
            source: unlocked ? "image://badges/" + badgeId : "image://badges/" + badgeLockedId
        }
        Text {
            Layout.row: 0
            Layout.column: 1
            Layout.fillWidth: true
            text : title
            font.pointSize: 20
        }
        Text {
            Layout.row: 1
            Layout.column: 1
            Layout.fillWidth: true
            Layout.fillHeight: true
            text : description
        }
    }
}
