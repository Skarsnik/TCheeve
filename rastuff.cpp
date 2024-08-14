#include "rastuff.h"
#include "ui_rastuff.h"
#include <logindialog.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_RAStuff, "RAStuff")
#define sDebug() qCDebug(log_RAStuff)
#define sInfo() qCInfo(log_RAStuff)

RAStuff::RAStuff(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RAStuff)
{
    ui->setupUi(this);
    tts = new QTextToSpeech(this);
    achChecker = new AchievementChecker(this);
    connect(achChecker, &AchievementChecker::achievementCompleted, this, &RAStuff::achievementCompleted);
    doRegularLogin("Skarsnik", "gobbla42");
    setUsb2Snes();
    connect(&raManager, &RAWebApiManager::loginSuccess, this, [=] {
        //nwaccess.connectToHost();
        usb2snes.connect();
    });
    connect(&raManager, &RAWebApiManager::gameIdGotten, this, [=] (int gameId) {
        raManager.getGameInfos(gameId);
    });
    connect(&raManager, &RAWebApiManager::gameInfosDone, this, &RAStuff::onGameInfosDone);
    connect(&nwaccess, &NWAccess::getMemoriesDone, this, &RAStuff::onGetMemoriesDone);

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
                sInfo() << d.name << d.size;
                if (d.name == "WRAM")
                    achChecker->allocRAM(d.size);
            }
            gameStarted(gameInfos.file);
        });
    checkInfoTimer.setInterval(500);
    auto voices = tts->availableVoices();
    quint32 rvalue = QRandomGenerator::global()->generate();
    tts->setVoice(voices[rvalue % voices.size()]);
    tts->say("RA Stuff started");
}

void RAStuff::setUsb2Snes()
{
    //usb2snes.connect();
    achChecker->allocRAM(0x20000);
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
            usb2snesGameStarted(infos.romPlayed);
        }
    });
    checkInfoTimer.setInterval(500);
    connect(&checkInfoTimer, &QTimer::timeout, this, [=] {
        usb2snes.infos();
    });
    connect(&usb2snes, &USB2snes::fileGet, this, [=] {
        QByteArray romData = usb2snes.getFileData();
        if (romData.size() & 512)
            romData = romData.mid(512);
        QString md5 = QCryptographicHash::hash(romData, QCryptographicHash::Md5).toHex();
        raManager.getGameId(md5);
    });
    connect(&usb2snes, &USB2snes::getAddressDataReceived, this, [=] {
        achChecker->checkAchievements(usb2snes.getAsyncAdressData());
        usb2snes.getAsyncAddress(*memoriesToCheck);
    });
}

void RAStuff::usb2snesGameStarted(QString game)
{
    usb2snes.getFile(game);
}

void RAStuff::onGameInfosDone()
{
    sInfo() << "RA Manager finished game infos";
    QString text;
    text += "Game Tile : " + raManager.gameInfos.title + "\n\n";
    text += "Achievements : \n";
    memoriesToCheck = achChecker->prepareCheck(raManager.gameInfos.rawAchievements);
    sInfo() << "Displaying achievement list";
    ui->listWidget->clear();
    for (const auto& ach : raManager.gameInfos.rawAchievements)
    {
        AchievementListItem* newListItem = new AchievementListItem(ach.title, ach.description, ui->listWidget);
        listAchWidget[ach.id] = newListItem;
        auto p = new QListWidgetItem();
        p->setSizeHint(newListItem->sizeHint());
        ui->listWidget->addItem(p);
        ui->listWidget->setItemWidget(p, newListItem);
        achievementsToCheck[ach.id] = &ach;
        text += "Name : " + ach.title + "\n";
        text += "\tDescription : " + ach.description + "\n";
    }
    //nwaccess.getMemories(*memoriesToCheck);
    usb2snes.getAsyncAddress(*memoriesToCheck);
    ui->plainTextEdit->setPlainText(text);
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
        sInfo() << "Error with file";
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
    ui->plainTextEdit->appendPlainText("Achievement completed ! " + achievementsToCheck[id]->title);
    sInfo() << "Achievement completed " << achievementsToCheck[id]->title;
    tts->say("Achievement completed : " + achievementsToCheck[id]->title);
    if (nwaccess.hasMessage())
    {
        nwaccess.message("Achievement completed ! " + achievementsToCheck[id]->title);
    }
    listAchWidget[id]->setDone();
}





