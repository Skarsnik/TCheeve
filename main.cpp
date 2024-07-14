#include "rastuff.h"


#include "sqlogging.h"

#include <SQApplication.h>
#include <QLocale>
#include <QTranslator>


int main(int argc, char *argv[])
{
    SQApplication a(argc, argv);

    installSQLogging("logs.txt", "debug-log.txt", true);
    RAStuff w;
    w.show();
    return a.exec();
}

