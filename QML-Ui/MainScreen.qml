import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import fr.nyo.RAEngine 1.0

Rectangle {
    width: 800
    height: 600

    GridLayout {
        id: grid
        anchors.fill: parent
        rows: 2
        columns: 2

        Text {
            id : statusText
            text: {
                switch (MainEngine.status)
                {
                case MainEngine.Status.None:
                    return qsTr("Nothing going on, please login");
                case MainEngine.Status.WaitingForUsb2Snes:
                    return qsTr("Waiting for a game to be loaded");
                case MainEngine.Status.GettingAchievementInfo:
                    return qsTr("Getting the achievements informations");
                case MainEngine.Status.SessionStarted:
                    return qsTr("Session started, happy hunting")
                }
            }
            horizontalAlignment: Qt.AlignHCenter
            Layout.row: 0
            Layout.columnSpan: 2
            Layout.column: 0
            Layout.minimumHeight: 20
            Layout.fillWidth: true
        }
        Button {
            Layout.row: 1
            Layout.column: 0
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
            id: loginButton
            text: qsTr("Login")
        }

        ListView {
            id: achievementListView
            Layout.row: 1
            Layout.column: 1
            Layout.alignment: Qt.AlignRight
            Layout.preferredWidth: 400
            //Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            flickableDirection: Flickable.VerticalFlick
            model: MainEngine.achievementsModel
            ScrollBar.vertical: ScrollBar {}
            delegate: AchievementDelegate {}
        }
    }
}
