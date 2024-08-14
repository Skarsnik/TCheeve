import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import fr.nyo.RAEngine 1.0


/*
 This the 'real' main windows,
 please use layout instead of hardcoded positionning
 so it act sane when resizing the windows
*/
Rectangle {
    width: 600
    height: 600
    LoginDialog {
        id : loginDialog
        onLoginDialogButtonClicked: MainEngine.login(login, password)
        Connections {
            target: MainEngine
            function onLoginDone(success) {
                if (success)
                {
                    loginDialog.accept();
                } else {
                    loginDialog.statusText = "Login failed";
                }
            }
        }
    }

    Row {
        spacing: 50
        Button {
            id: loginButton
            text: qsTr("Login")
            onClicked: () => loginDialog.open()
        }
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
        }
    }

    ListView {
        id: achievementListView
        clip: true
        width: parent.width
        height: 500
        y : 50
        x : 20
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        model: MainEngine.achievementsModel
        ScrollBar.vertical: ScrollBar {}
        delegate: AchievementDelegate {}
    }
}
