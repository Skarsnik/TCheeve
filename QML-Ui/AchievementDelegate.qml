/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

/*
  This is actually where an achievement 'tile' is defined
 */


Rectangle {
    // This list all the ach properpy
    required property int achId
    required property string title
    required property string description
    required property int rarity
    required property int rarityHardcore
    required property int points
    required property bool unlocked
    required property bool hardcoreUnlocked
    required property date unlockedTime
    required property string badgeId
    required property string badgeLockedId
    required property bool official

    width : 400
    height : 80
    color: {
        var munlocked = false;
        if (hardcoreMode)
            munlocked = hardcoreUnlocked;
        else
            munlocked = unlocked;
        return munlocked ? "yellow" : "white"
    }
    GridLayout {
        columns : 2
        rows : 3
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
