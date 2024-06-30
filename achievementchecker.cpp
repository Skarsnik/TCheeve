#include "achievementchecker.h"
#include <QDebug>

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
    rc_condset_t* condset;
    char buffer[10000];

    m_memoriesToCheck.clear();
    cheevosCondset.clear();
    cheevosMemRefs.clear();
    for (const Achievement& ach : achievements)
    {
        if (ach.id != 959)
            continue;
        QByteArray bTmp = ach.memAddrString.toLocal8Bit();
        rc_condset_memrefs_t* memrefs = (rc_condset_memrefs_t*) malloc(sizeof(rc_condset_memrefs_t));
        memrefs->memrefs = (rc_memref_t*) malloc(sizeof(rc_memref_t));
        /*memrefs->memrefs->address = 0;
        memrefs->memrefs->next = 0;
        memrefs->memrefs->value.value = 0;
        memrefs->memrefs->value.changed = 0;
        memrefs->memrefs->value.is_indirect = 0;
        memrefs->memrefs->value.prior = 0;
        memrefs->memrefs->value.size = 0;
        memrefs->memrefs->value.type = 0;*/
        const char* memaddr = bTmp.constData();
        qDebug() << ach.id  << memaddr;
        rc_parse_state_t parse;

        rc_init_parse_state(&parse, buffer, 0, 0);
        rc_init_parse_state_memrefs(&parse, &memrefs->memrefs);
        rc_init_parse_state_variables(&parse, &memrefs->variables);
        condset = rc_parse_condset(&memaddr, &parse, 0);
        rc_destroy_parse_state(&parse);
        cheevosMemRefs[ach.id] = memrefs;
        cheevosCondset[ach.id] = condset;
        qDebug() << "Mmemrefs in build" << memrefs;
        rc_memref_t* m_next = memrefs->memrefs;
        while (m_next != nullptr)
        {
            qDebug() << "Memref->memref " << m_next;
            m_memoriesToCheck.append(QPair<unsigned int, unsigned int>(m_next->address, sizeOfCheevosEnum(m_next->value.size)));
            m_next = m_next->next;
        }
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
    /*qDebug() << "Duplicate : " << m_memoriesToCheck;
    qDebug() << "==============================\n==========================\n";*/
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
    /*qDebug() << "==============================\n==========================\n";
    qDebug() << "Some mergin done" << m_memoriesToCheck;*/
    qDebug() << "Number of read " << m_memoriesToCheck.size();
    qDebug() << "After build Address : " << QString::number(cheevosMemRefs[959]->memrefs->address, 16);
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

    qDebug() << "begining check memrefs->memrefs" << QString::number(cheevosMemRefs[959]->memrefs->address, 16);
    unsigned int pos = 0;
    for (const auto& mem : m_memoriesToCheck)
    {
        memcpy(virtualRAM + mem.first, datas + pos, mem.second);
    }
    for (const auto& key : cheevosCondset.keys())
    {
        qDebug() << "Checking " << key;
        rc_condset_t* condset = cheevosCondset[key];
        rc_condset_memrefs_t* memrefs = cheevosMemRefs[key];
        rc_eval_state_t eval_state;

        qDebug() << "Checking memrefs->memrefs" << memrefs->memrefs;
        qDebug() << "Check Address : " << QString::number(memrefs->memrefs->address, 16);
        rc_update_memref_values(memrefs->memrefs, peek, virtualRAM);
        rc_update_variables(memrefs->variables, peek, virtualRAM, 0);

        memset(&eval_state, 0, sizeof(eval_state));
        eval_state.peek = peek;
        eval_state.peek_userdata = virtualRAM;

        int result = rc_test_condset(condset, &eval_state);
        if (result == 1)
        {
            emit achievementCompleted(key);
        }
    }
}

QList<QPair<int, int> > *AchievementChecker::memoriesToCheck()
{
    return &m_memoriesToCheck;
}


