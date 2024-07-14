#include "achievementchecker.h"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(log_AchChecker, "RAManager")
#define sDebug() qCDebug(log_AchChecker)
#define sInfo() qCInfo(log_AchChecker)

AchievementChecker::AchievementChecker(QObject *parent)
    : QObject{parent}
{}

void AchievementChecker::allocRAM(size_t size)
{
    qDebug() << "Allocating virtual ram" << size;
    virtualRAM = (char*) malloc(size * sizeof(char));
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

QList<QPair<int, int> > *AchievementChecker::prepareCheck(QList<Achievement> &achievements)
{
    cheevosCondset.clear();
    cheevosMemRefs.clear();
    m_memrefs = (rc_memref_t*) malloc(sizeof(m_memrefs));
    memset(m_memrefs, 0, sizeof(m_memrefs));
    for (const Achievement& ach : achievements)
    {
        QByteArray bTmp = ach.memAddrString.toLocal8Bit();
        const char* memaddr = bTmp.constData();
        //rc_parse_state_t    parse;
        rc_trigger_t*       new_trigger;
        size_t              size_alloc = rc_trigger_size(memaddr);
        void*               trigger_buffer = malloc(size_alloc);
        //rc_init_parse_state(&parse, trigger_buffer, nullptr, 0);
        //parse.first_memref = 0;
        //new_trigger = RC_ALLOC(rc_trigger_t, &parse);
        new_trigger = rc_parse_trigger(trigger_buffer, memaddr, NULL, 0);
        cheevosTriggers[ach.id] = new_trigger;
        //rc_destroy_parse_state(&parse);
        rc_memref_t* m_next = new_trigger->memrefs;
        sDebug() << "String : " << memaddr;
        while (m_next != nullptr)
        {
            sDebug() << "Address " << m_next->address;
            m_achievementsMemLists[ach.id].append(QPair<unsigned int, unsigned int>(m_next->address, sizeOfCheevosEnum(m_next->value.size)));
            //m_memoriesToCheck.append(QPair<unsigned int, unsigned int>(m_next->address, sizeOfCheevosEnum(m_next->value.size)));
            m_next = m_next->next;
        }
        //printDebug("Build achievement");
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
    return &m_memoriesToCheck;
}


static uint32_t peek(uint32_t address, uint32_t num_bytes, void* ud) {

    const char* mem = (const char*) ud;
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

    unsigned int pos = 0;
    for (const auto& mem : m_memoriesToCheck)
    {
        memcpy(virtualRAM + mem.first, datas + pos, mem.second);
        pos += mem.second;
    }
    for (const auto& key : cheevosTriggers.keys())
    {
        rc_trigger_t* trigger = cheevosTriggers[key];
        rc_test_trigger(trigger, peek, virtualRAM, nullptr);
        if (trigger->state == RC_TRIGGER_STATE_TRIGGERED)
        {
            sInfo() << key << "Achievement achieved " << key;
            m_achievementsMemLists.remove(key);
            cheevosTriggers.remove(key);
            buildMemoryChecks();
            emit achievementCompleted(key);
        }
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



