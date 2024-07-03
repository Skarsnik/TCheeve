#include "rastuff.h"
#include "ui_rastuff.h"
#include <logindialog.h>

RAStuff::RAStuff(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RAStuff)
{
    ui->setupUi(this);
    achChecker = new AchievementChecker(this);
    connect(achChecker, &AchievementChecker::achievementCompleted, this, &RAStuff::achievementCompleted);
    //doRegularLogin("Skarsnik", "gobbla42");
    connect(&raManager, &RAManager::loginSuccess, this, [=] {
        nwaccess.connectToHost();
    });
    connect(&raManager, &RAManager::gameIdGotten, this, [=] (int gameId) {
        raManager.getGameInfos(gameId);
    });
    connect(&raManager, &RAManager::gameInfosDone, this, &RAStuff::onGameInfosDone);
    connect(&nwaccess, &NWAccess::getMemoriesDone, this, &RAStuff::onGetMemoriesDone);
    /*
    usb2snes.connect();
    QTimer::singleShot(500, this, [=] {
        if (usb2snes.state() == USB2snes::None)
        {
            usb2snes.connect();
        }
    });
    connect(&usb2snes, &USB2snes::stateChanged, this, &RAStuff::onUsb2SnesStateChanged);
    connect(&usb2snes, &USB2snes::infoReceived, this, [=](Usb2SnesInfo infos) {
        if (infos.isMenu == false)
        {
            checkInfoTimer.stop();
            gameStarted(infos.romPlayed);
        }
    });
    checkInfoTimer.setInterval(500);
    connect(&checkInfoTimer, &QTimer::timeout, this, [=] {
        usb2snes.infos();
    });*/

    connect(&nwaccess, &NWAccess::ready, this, [=] {
        ui->nwaStatusLabel->setText("Connected to " + nwaccess.emulatorInfos().name);
        nwaccess.requestGameInfos();
    });
    connect(&nwaccess, &NWAccess::gameInfosDone, this, [=] {
        if (nwaccess.gameInfos().name.isEmpty())
        {
            ui->nwaStatusLabel->setText(nwaccess.emulatorInfos().name + " not running a game");
            checkInfoTimer.start();
        } else {
            checkInfoTimer.stop();
            ui->nwaStatusLabel->setText(nwaccess.emulatorInfos().name + " running " + nwaccess.gameInfos().name);
            gameInfos = nwaccess.gameInfos();
            nwaccess.requestMemoriesDomains();
        }
    });
    connect(&checkInfoTimer, &QTimer::timeout, this, [=] {
        nwaccess.requestGameInfos();
    });

    // TODO, this while part is not great x)
    connect(&nwaccess, &NWAccess::memoriesDomainsDone, this, [=] {
            auto domains = nwaccess.memoriesDomains();
            for (const auto& d : domains)
            {
                qDebug() << d.name << d.size;
                if (d.name == "WRAM")
                    achChecker->allocRAM(d.size);
            }
            gameStarted(gameInfos.file);
        });
    checkInfoTimer.setInterval(500);
}

void RAStuff::onGameInfosDone()
{
    qDebug() << "RA Manager finished game infos";
    QString text;
    text += "Game Tile : " + raManager.gameInfos.title + "\n\n";
    text += "Achievements : \n";
    memoriesToCheck = achChecker->prepareCheck(raManager.gameInfos.achievements);
    achChecker->printDebug("After prepare check call");
    for (const auto& ach : raManager.gameInfos.achievements)
    {
        achievementsToCheck[ach.id] = &ach;
        text += "Name : " + ach.title + "\n";
        text += "\tDescription : " + ach.description + "\n";
    }
    achChecker->printDebug("Before get memories");
    nwaccess.getMemories(*memoriesToCheck);
    achChecker->printDebug("After get memories");
    ui->plainTextEdit->setPlainText(text);
    achChecker->printDebug("End of GameInfo done");

}

void RAStuff::onGetMemoriesDone()
{
    achChecker->checkAchievements(nwaccess.memoryDatas());
    nwaccess.getMemories(*memoriesToCheck);
}

RAStuff::~RAStuff()
{
    delete ui;
}

void RAStuff::on_pushButton_clicked()
{
    LoginDialog diag;
    if (diag.exec() )
    {
        doRegularLogin(diag.login, diag.password);
    }
}

void RAStuff::doRegularLogin(QString login, QString password)
{
    raManager.regularLogin(login, password);
}

void RAStuff::onUsb2SnesStateChanged()
{
    auto state = usb2snes.state();
    if (state == USB2snes::Ready)
    {
        checkInfoTimer.start();
    }
}


// This is for NWA
void RAStuff::gameStarted(QString romPath)
{
    QFile gameFile(romPath);
    bool    header = false;
    QFileInfo fi(romPath);
    if (fi.size() & 512)
    {
        header = true;
    }
    if (gameFile.open(QIODevice::ReadOnly) == false)
    {
        qDebug() << "Error with file";
        return ;
    }
    QByteArray romData = gameFile.readAll();
    if (header)
        romData = romData.mid(512);
    QString md5 = QCryptographicHash::hash(romData, QCryptographicHash::Md5).toHex();
    raManager.getGameId(md5);
}

void RAStuff::achievementCompleted(int id)
{
    ui->plainTextEdit->appendPlainText("Achievement completed !" + achievementsToCheck[id]->title);
    if (nwaccess.hasMessage())
    {
        nwaccess.message("Achievement completed !" + achievementsToCheck[id]->title);
    }
}





