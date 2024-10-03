#ifndef ACHIEVEMENTCHECKER_H
#define ACHIEVEMENTCHECKER_H

#include <QObject>
#include <QMap>
#include "memoryviewer.h"
#include "rastruct.h"
#include <rc_internal.h>

// This class handle all the achievement check and such
// contains all the rcheevos stuff

class AchievementChecker : public QObject
{
    Q_OBJECT
public:

    enum TriggerState {
        RC_TRIGGER_STATE_INACTIVE,   /* achievement is not being processed */
        RC_TRIGGER_STATE_WAITING,    /* achievement cannot trigger until it has been false for at least one frame */
        RC_TRIGGER_STATE_ACTIVE,     /* achievement is active and may trigger */
        RC_TRIGGER_STATE_PAUSED,     /* achievement is currently paused and will not trigger */
        RC_TRIGGER_STATE_RESET,      /* achievement hit counts were reset */
        RC_TRIGGER_STATE_TRIGGERED,  /* achievement has triggered */
        RC_TRIGGER_STATE_PRIMED,     /* all non-Trigger conditions are true */
        RC_TRIGGER_STATE_DISABLED
    };
    Q_ENUM(TriggerState)

    explicit    AchievementChecker(QObject *parent = nullptr);

    void                            allocRAM(size_t size);
    QList<QPair<int, int> >*        prepareCheck(QList<RawAchievement>& achievements);
    void                            checkAchievements(const QByteArray& bdatas);
    QList<QPair<int, int> >*        memoriesToCheck();
    void                            printDebug(QString where);

signals:
    void    achievementCompleted(unsigned int id);
    void    achievementPrimed(unsigned int id);
    void    achievementUnprimed(unsigned int id);

private:

    typedef struct rc_condset_memrefs_t
    {
        rc_memref_t* memrefs;
        rc_value_t* variables;
    } rc_condset_memrefs_t;

    rc_memref_t*                                m_memrefs;
    quint8*                                     virtualRAM;
    QMap<unsigned int, QList<QPair<int, int> > > m_achievementsMemLists;
    QList<QPair<int, int> >                     m_memoriesToCheck;
    QMap<unsigned int, rc_condset_t*>           cheevosCondset;
    QMap<unsigned int, rc_condset_memrefs_t*>   cheevosMemRefs;
    QMap<unsigned int, rc_trigger_t*>           cheevosTriggers;
    QMap<unsigned int, TriggerState>            previousState;
    MemoryViewer*                               memView;

    QList<QPair<int, int> > * buildMemoryChecks();
};

#endif // ACHIEVEMENTCHECKER_H
