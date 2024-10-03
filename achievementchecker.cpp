#include "achievementchecker.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_AchChecker, "RAManager")
#define sDebug() qCDebug(log_AchChecker)
#define sInfo() qCInfo(log_AchChecker)

AchievementChecker::AchievementChecker(QObject *parent)
    : QObject{parent}
{
    memView = new MemoryViewer();
}

void AchievementChecker::allocRAM(size_t size)
{
    qDebug() << "Allocating virtual ram" << size;
    virtualRAM = (quint8*) malloc(size * sizeof(char));
    memView->setVirtualRAM(virtualRAM);
    //memView->show();
}

static unsigned int sizeOfCheevosEnum(uint8_t memType)
{
    switch(memType)
    {
    case RC_MEMSIZE_8_BITS:
    case RC_MEMSIZE_BIT_0:
    case RC_MEMSIZE_BIT_1:
    case RC_MEMSIZE_BIT_2:
    case RC_MEMSIZE_BIT_3:
    case RC_MEMSIZE_BIT_4:
    case RC_MEMSIZE_BIT_5:
    case RC_MEMSIZE_BIT_6:
    case RC_MEMSIZE_BIT_7:
        return 1;
    case RC_MEMSIZE_16_BITS:
    case RC_MEMSIZE_16_BITS_BE:
        return 2;
    case RC_MEMSIZE_32_BITS:
    case RC_MEMSIZE_32_BITS_BE:
    case RC_MEMSIZE_VARIABLE:
        return 4;
    default: // In doubt read 4?
        return 4;

    }
}

QList<QPair<int, int> > *AchievementChecker::prepareCheck(QList<RawAchievement> &achievements)
{
    cheevosCondset.clear();
    cheevosMemRefs.clear();
    previousState.clear();

    for (const RawAchievement& ach : achievements)
    {
        if (ach.unlocked)
            continue;
        QByteArray bTmp = ach.memAddrString.toLocal8Bit();
        previousState[ach.id] = RC_TRIGGER_STATE_INACTIVE;
        const char* memaddr = bTmp.constData();
        rc_trigger_t*       new_trigger;
        size_t              size_alloc = rc_trigger_size(memaddr);
        void*               trigger_buffer = malloc(size_alloc);
        new_trigger = rc_parse_trigger(trigger_buffer, memaddr, NULL, 0);
        cheevosTriggers[ach.id] = new_trigger;
        rc_memref_t* m_next = new_trigger->memrefs;
        //sDebug() << "String : " << memaddr << ach.id << ach.title;
        while (m_next != nullptr)
        {
            //sDebug() << "\t Address " << m_next->address;
            m_achievementsMemLists[ach.id].append(QPair<unsigned int, unsigned int>(m_next->address, sizeOfCheevosEnum(m_next->value.size)));
            m_next = m_next->next;
        }
    }
    return buildMemoryChecks();
}

QList<QPair<int, int> >* AchievementChecker::buildMemoryChecks()
{
    m_memoriesToCheck.clear();
    for (const auto& key : m_achievementsMemLists.keys())
    {
        m_memoriesToCheck.append(m_achievementsMemLists[key]);
    }
    std::sort(m_memoriesToCheck.begin(), m_memoriesToCheck.end(), [&](QPair<unsigned int, unsigned int> p1, QPair<unsigned int, unsigned int> p2) {
        return p1.first < p2.first;
    });
    for (unsigned int i = 0; i < m_memoriesToCheck.size() - 1; i++)
    {
        const auto& currentPair = m_memoriesToCheck.at(i);
        const auto& nextPair = m_memoriesToCheck.at(i + 1);
        if (currentPair.first == nextPair.first)
        {
            m_memoriesToCheck[i].second = qMax(currentPair.second, nextPair.second);
            m_memoriesToCheck.remove(i + 1);
            i--;
            continue;
        }
    }
    sInfo() << "Building memory to check";
    sInfo() << "==============================\n==========================\n";
    for (unsigned int i = 0; i < m_memoriesToCheck.size() - 1; i++)
    {
        const auto& currentPair = m_memoriesToCheck.at(i);
        const auto& nextPair = m_memoriesToCheck.at(i + 1);

        // We overshoot in hope that merge more
        if (currentPair.first + currentPair.second >= nextPair.first - 1)
        {
            m_memoriesToCheck[i].second += nextPair.second + 1;
            m_memoriesToCheck.remove(i + 1);
            i--;
            continue;
        }
    }
    sInfo() << "Some mergin done" << m_memoriesToCheck;
    sInfo() << "==============================\n==========================\n";
    sInfo() << "Number of read " << m_memoriesToCheck.size();
    memView->setMemoryArea(m_memoriesToCheck);
    return &m_memoriesToCheck;
}


static uint32_t peek(uint32_t address, uint32_t num_bytes, void* ud) {

    const quint8* mem = (const quint8*) ud;
    switch (num_bytes) {
    case 1: return mem[address];

    case 2: return mem[address] |
               mem[address + 1] << 8;

    case 4: return mem[address] |
               mem[address + 1] << 8 |
               mem[address + 2] << 16 |
               mem[address + 3] << 24;
    }

    return 0;
}


void AchievementChecker::checkAchievements(const QByteArray& bdatas)
{
    const char* datas = bdatas.constData();
    //static int nbHits = 0;

    unsigned int pos = 0;
    for (const auto& mem : m_memoriesToCheck)
    {
        memcpy(virtualRAM + mem.first, datas + pos, mem.second);
        pos += mem.second;
    }
    memView->memoryUpdated();
    for (const auto& key : cheevosTriggers.keys())
    {
        rc_trigger_t* trigger = cheevosTriggers[key];
        rc_test_trigger(trigger, peek, virtualRAM, nullptr);
        /*if (virtualRAM[0x57f] == 0x90)
            nbHits++;
        if (virtualRAM[0x57f] == 0x90)
        {
            sDebug() << "Hit the 0x57f == 0x90" << nbHits;
        }*/
        TriggerState state = static_cast<TriggerState>(trigger->state);
        if (state == RC_TRIGGER_STATE_TRIGGERED)
        {
            sInfo() << key << "Achievement achieved " << key;
            m_achievementsMemLists.remove(key);
            cheevosTriggers.remove(key);
            buildMemoryChecks();
            emit achievementCompleted(key);
        }
        if (state == RC_TRIGGER_STATE_PRIMED && previousState[key] != RC_TRIGGER_STATE_PRIMED)
        {
            sInfo() << key << "Achievement primed" << key;
            emit achievementPrimed(key);
        }
        if (previousState[key] == RC_TRIGGER_STATE_PRIMED
            && state != RC_TRIGGER_STATE_PRIMED
            && state != RC_TRIGGER_STATE_TRIGGERED)
        {
            sInfo() << "Unprimed";
            emit achievementUnprimed(key);
        }
        previousState[key] = state;
    }
}

QList<QPair<int, int> > *AchievementChecker::memoriesToCheck()
{
    return &m_memoriesToCheck;
}

void AchievementChecker::printDebug(QString where)
{
    qDebug() << where << QString::number(cheevosTriggers[959]->memrefs->address, 16);
}



