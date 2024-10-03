import QtQuick 2.15
import QtQuick.Controls
import fr.nyo.TCEngine 1.0
import QtTextToSpeech
import QtMultimedia

Window {
    property bool hardcoreMode : MainEngine.hardcoreMode
    property list<string> soundQueue
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "Thanks Cheeve"

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
            tts.say("RetroAchievements session started");
        }
        // When an achievement is unlocked
        function onAchievementAchieved(achievement) {
            console.log("Achievement unlocked " + achievement.title);
            tts.say("Achievement unlocked : " + achievement.title);
            playSound("qrc:/Sounds/rooster-cry-173621.mp3");
        }
        // When an achievement is primed
        function onAchievementPrimed(achievement) {
            console.log("Achievement primed " + achievement.title);
            tts.say("Achievement primed : " + achievement.title);
            playSound("qrc:/Sounds/chicken-noise-196746.mp3");
        }
        // When an achievement get unprimed
        function onAchievementUnprimed(achievement) {
            console.log("Achievement unprimed " + achievement.title);
            tts.say("Achievement unprimed");
            playSound("qrc:/Sounds/chicken-noise-228106.mp3");
        }

        // When the logged request is done,
        // Check success to see if logged or not
        // Note that the login dialog already use this
        function onLoginDone(success) {
            mainScreen.logged = success
        }
    }
    MediaPlayer {
        id : mediaPlayer
        videoOutput : {}
        audioOutput : AudioOutput {}
        onPlaybackStateChanged: {
            if (playbackState === MediaPlayer.StoppedState)
            {
               if (soundQueue.length != 0)
               {
                   mediaPlayer.source = soundQueue.pop();
                   mediaPlayer.play()
               }
            }
        }
    }
    function playSound(soundUrl) {
        console.log(mediaPlayer.playbackState)
        if (mediaPlayer.playbackState === MediaPlayer.StoppedState)
        {
            mediaPlayer.source = soundUrl;
            mediaPlayer.play();
        }
        else
            soundQueue.push(soundUrl);
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
