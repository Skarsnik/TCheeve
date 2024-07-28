#include "rastuff.h"


#include "raengine.h"
#include "sqlogging.h"
#include <QQmlContext>

#include <SQApplication.h>
#include <QLocale>
#include <QTranslator>
#include <QQmlApplicationEngine>
#include "sharedstruct.h"


int main(int argc, char *argv[])
{
    SQApplication a(argc, argv);

    installSQLogging("logs.txt", "debug-log.txt", true);
    QLoggingCategory::setFilterRules("USB2SNES.debug=false");

    QQmlApplicationEngine engine;
    RAEngine raEngine;

    engine.addImportPath("qrc:/QML-Ui/");
    engine.rootContext()->setContextProperty("mainEngine", &raEngine);
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

