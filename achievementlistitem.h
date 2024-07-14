#ifndef ACHIEVEMENTLISTITEM_H
#define ACHIEVEMENTLISTITEM_H

#include <QWidget>

namespace Ui {
class AchievementListItem;
}

class AchievementListItem : public QWidget
{
    Q_OBJECT

public:
    explicit AchievementListItem(QString title, QString description, QWidget *parent = nullptr);
    void     setDone();
    ~AchievementListItem();

private:
    Ui::AchievementListItem *ui;
};

#endif // ACHIEVEMENTLISTITEM_H
