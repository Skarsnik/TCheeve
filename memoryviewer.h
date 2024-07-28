#ifndef MEMORYVIEWER_H
#define MEMORYVIEWER_H

#include <QWidget>

namespace Ui {
class MemoryViewer;
}

class MemoryViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MemoryViewer(QWidget *parent = nullptr);
    void    setVirtualRAM(quint8* vRAM);
    void    setMemoryArea(QList<QPair<int, int> > mems);
    ~MemoryViewer();
    void    memoryUpdated();

private:
    Ui::MemoryViewer *ui;
    quint8*       virtualRAM;
    QList<QPair<int, int> > memoryAreas;
};

#endif // MEMORYVIEWER_H
