#include "memoryviewer.h"
#include "ui_memoryviewer.h"

MemoryViewer::MemoryViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MemoryViewer)
{
    ui->setupUi(this);
}

void MemoryViewer::setVirtualRAM(quint8 *vRAM)
{
    virtualRAM = vRAM;
}

void MemoryViewer::setMemoryArea(QList<QPair<int, int> > mems)
{
    memoryAreas = mems;
}

MemoryViewer::~MemoryViewer()
{
    delete ui;
}

void MemoryViewer::memoryUpdated()
{
    ui->textEdit->clear();
    for (const auto &mems : memoryAreas)
    {
        unsigned int offset = mems.first;
        unsigned int size = mems.second;
        QString toAdd;
        QString header = QString("%1 (%2) : ").arg(offset, 6, 16, QChar('0')).arg(size, 3, 10, QChar('0'));
        toAdd = header;
        for (unsigned int i = 0; i < size; i++)
        {
            toAdd.append(QString("%1 ").arg(virtualRAM[offset + i], 2, 16, QChar('0')));
        }
        toAdd.append("\n");
        ui->textEdit->append(toAdd);
    }
}
