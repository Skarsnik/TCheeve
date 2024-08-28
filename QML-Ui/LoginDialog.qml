import QtQuick 2.15
import QtQuick.Controls
import fr.nyo.TCEngine 1.0

Dialog {
    title: "Login Dialog"
    modal: true
    signal loginDialogButtonClicked();
    property string login : loginText.text
    property string password : passwordText.text
    property bool   rememberMe : MainEngine.rememberLogin
    property string statusText : " "

    contentItem: Column {
        Row {
            spacing: 5
            Text {
                text : "Login"
            }

            TextField {
                width: 50
                id: loginText
            }

        }
        Row {
            spacing: 5
            Text {
                text : qsTr("Password")
            }

            TextField {
                width: 50
                id: passwordText
                echoMode: TextInput.Password
            }
        }
        CheckBox {
            text : qsTr("Remember me")
            id: rememberLoginCheckBox
            checked : rememberMe
            onCheckedChanged: {
                rememberMe = checked;
                MainEngine.rememberLogin = checked;
            }
        }

        Text {
            text : statusText
            color : "red"
        }
    }
    footer: DialogButtonBox {
        Button {
            id: loginDialogButton
            text: qsTr("Login")
            onClicked : loginDialogButtonClicked()
        }
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
}
