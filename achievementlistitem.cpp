#include "achievementlistitem.h"
#include "ui_achievementlistitem.h"

AchievementListItem::AchievementListItem(QString title, QString description, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AchievementListItem)
{
    ui->setupUi(this);
    ui->nameLabel->setText(QString("<h2>%1</h2>").arg(title));
    ui->descriptionLabel->setText(description);
}

void AchievementListItem::setDone()
{
    QPalette pal = QPalette();
    pal.setColor(QPalette::Base, Qt::yellow);
    setAutoFillBackground(true);
    setPalette(pal);
}

AchievementListItem::~AchievementListItem()
{
    delete ui;
}
