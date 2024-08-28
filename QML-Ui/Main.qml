import QtQuick 2.15
import QtQuick.Controls
import fr.nyo.RAEngine 1.0
import QtTextToSpeech

Window {
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "RAStuff - Yay"

    MainScreen {
        id: mainScreen
    }

    /* You communicate with the logic via the MainEgine keyword
    / Take a look at the raengine.h, you can ignore most thing in there
    / thing declared like Q_PROPERTY(blabla) are what property you have access
    / For exemple you can see
     Q_PROPERTY(Status status READ status NOTIFY statusChanged FINAL)
     that mean you have access to the MainEngine.status property

    Q_INVOKABLE marks methods you can call to do some action.
    For example you have the the login method to log.

    // This are the signals (events) that can happen
    */
    Connections {
        target : MainEngine

        // The session is started
        function onSessionStarted() {
            mainScreen.hardcoreChoiceEnabled = false;
            tts.say("RetroAchievements session started");
        }
        // When an achievement is unlocked
        function onAchievementAchieved(achievement) {
            console.log("Achievement unlocked " + achievement.title);
            tts.say("Achievement unlocked : " + achievement.title);
        }
        // When the logged request is done,
        // Check success to see if logged or not
        // Note that the login dialog already use this
        function onLoginDone(success) {
            mainScreen.logged = success
        }
    }
    TextToSpeech {
        id : tts
        // This is called more or less on init
        // Here we pick a random voice
        Component.onCompleted: {
            var voices = tts.availableVoices();
            tts.voice = voices[Math.floor(Math.random() * voices.length)];
        }
    }
}
