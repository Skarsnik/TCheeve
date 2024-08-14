
#include "badgeimageprovider.h"
#include "raengine.h"
#include "sqlogging.h"
#include <QQmlContext>

#include <SQApplication.h>
#include <QLocale>
#include <QTranslator>
#include <QQmlApplicationEngine>



int main(int argc, char *argv[])
{
    SQApplication a(argc, argv);

    installSQLogging("logs.txt", "debug-log.txt", true);
    QLoggingCategory::setFilterRules("USB2SNES.debug=false");

    QQmlApplicationEngine engine;
    QScopedPointer<RAEngine> raEngine(new RAEngine);

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

