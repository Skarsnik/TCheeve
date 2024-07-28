#ifndef MEMORYVIEWWIDGET_H
#define MEMORYVIEWWIDGET_H

#include <QWidget>

class MemoryViewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MemoryViewWidget(QWidget *parent = nullptr);


signals:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
};

#endif // MEMORYVIEWWIDGET_H
