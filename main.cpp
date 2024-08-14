
#include "badgeimageprovider.h"
#include "raengine.h"
#include "sqlogging.h"
#include <QQmlContext>

#include <SQApplication.h>
#include <QLocale>
#include <QTranslator>
#include <QQmlApplicationEngine>

void    fillAchievementList (RAEngine* engine);

int main(int argc, char *argv[])
{
    SQApplication a(argc, argv);

    installSQLogging("logs.txt", "debug-log.txt", true);
    QLoggingCategory::setFilterRules("USB2SNES.debug=false");

    QQmlApplicationEngine engine;
    QScopedPointer<RAEngine> raEngine(new RAEngine);

    fillAchievementList(raEngine.get());
    engine.addImportPath("qrc:/QML-Ui/");
    qmlRegisterSingletonInstance("fr.nyo.RAEngine", 1, 0, "MainEngine", raEngine.get());
    engine.addImageProvider(QLatin1String("badges"), raEngine->bagdgeImageProvider());
    const QUrl url(QStringLiteral("qrc:/QML-Ui/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &a,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);
    /*RAStuff w;
    w.show();*/
    return a.exec();
}

void    fillAchievementList (RAEngine* engine)
{
    Achievement ach;
    ach.id = 78;
    ach.title = "My first achievement";
    ach.description = "Open RAStuff for the first time";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = true;
    ach.unlockedTime = QDateTime::fromString("2024-06-12 20:00");
    ach.badgeId = "78";
    ach.badgeLockedId = "78_lock";
    engine->testAddAchievement(ach);

    ach.id = 100;
    ach.title = "Praise the kottShicken";
    ach.description = "Praise the kottShicken once a day";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = true;
    ach.unlockedTime = QDateTime::fromString("2024-06-20 20:00");
    ach.badgeId = "100";
    ach.badgeLockedId = "100_lock";
    engine->testAddAchievement(ach);

    ach.id = 200;
    ach.title = "Praise the skarsShicken";
    ach.description = "Praise the skarsShicken once a day";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = false;
    ach.unlockedTime = QDateTime::fromString("2024-06-20 20:00");
    ach.badgeId = "100";
    ach.badgeLockedId = "100_lock";
    engine->testAddAchievement(ach);

    ach.id = 400;
    ach.title = "Give me the twitch money";
    ach.description = "Subscribe to Skarsnikus twitch channel";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = false;
    ach.unlockedTime = QDateTime::fromString("2024-06-20 20:00");
    ach.badgeId = "400";
    ach.badgeLockedId = "400_lock";
    engine->testAddAchievement(ach);

    ach.id = 500;
    ach.title = "Give me the github money";
    ach.description = "Sponsor to Skarsnik github account";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = false;
    ach.unlockedTime = QDateTime::fromString("2024-06-20 20:00");
    ach.badgeId = "500";
    ach.badgeLockedId = "500_lock";
    engine->testAddAchievement(ach);

    ach.id = 550;
    ach.title = "Don't do the above";
    ach.description = "Keep your money";
    ach.author = "Skarsnik";
    ach.rarity = 0;
    ach.rarityHardcore = 0;
    ach.points = 4;
    ach.unlocked = false;
    ach.unlockedTime = QDateTime::fromString("2024-06-20 20:00");
    ach.badgeId = "550";
    ach.badgeLockedId = "550_lock";
    engine->testAddAchievement(ach);


}
