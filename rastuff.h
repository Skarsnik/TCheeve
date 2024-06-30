#ifndef RASTUFF_H
#define RASTUFF_H

#include "achievementchecker.h"
#include "nwaccess.h"
#include "ramanager.h"
#include "usb2snes.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class RAStuff;
}
QT_END_NAMESPACE

class RAStuff : public QMainWindow
{
    Q_OBJECT

public:
    RAStuff(QWidget *parent = nullptr);
    ~RAStuff();

private slots:
    void on_pushButton_clicked();

private:
    Ui::RAStuff *ui;
    void doRegularLogin(QString, QString);

    /*struct      TrackedMemory {
        unsigned int    address;
        unsigned int    size;
    };*/
    bool        logged;
    QString     authToken;
    QString     login;
    USB2snes    usb2snes;
    NWAccess    nwaccess;
    RAManager   raManager;
    QTimer      checkInfoTimer;

    AchievementChecker*                     achChecker;
    QList<QPair<int, int> >*                memoriesToCheck;

    //QMap<unsigned int, TrackedMemory>       trackedMems;
    QMap<unsigned int, const Achievement*>  achievementsToCheck;
    char*                                   virtualRam;
    NWAGameInfos                            gameInfos;

    void    onUsb2SnesStateChanged();
    void    gameStarted(QString romPath);
    void    achievementCompleted(int id);
    void    onGameInfosDone();
    void    onGetMemoriesDone();
};
#endif // RASTUFF_H
