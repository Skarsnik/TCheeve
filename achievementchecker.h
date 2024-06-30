#ifndef ACHIEVEMENTCHECKER_H
#define ACHIEVEMENTCHECKER_H

#include <QObject>
#include <QMap>
#include "rastruct.h"
#include <rc_internal.h>

// This class handle all the achievement check and such
// contains all the rcheevos stuff

class AchievementChecker : public QObject
{
    Q_OBJECT
public:
    explicit    AchievementChecker(QObject *parent = nullptr);

    void                            allocRAM(size_t size);
    QList<QPair<int, int> >*        prepareCheck(QList<Achievement>& achievements);
    void                            checkAchievements(const QByteArray& bdatas);
    QList<QPair<int, int> >*        memoriesToCheck();

signals:
    void    achievementCompleted(unsigned int id);

private:

    typedef struct rc_condset_memrefs_t
    {
        rc_memref_t* memrefs;
        rc_value_t* variables;
    } rc_condset_memrefs_t;

    char*                                       virtualRAM;
    QList<QPair<int, int> >                     m_memoriesToCheck;
    QMap<unsigned int, rc_condset_t*>           cheevosCondset;
    QMap<unsigned int, rc_condset_memrefs_t*>   cheevosMemRefs;
};

#endif // ACHIEVEMENTCHECKER_H
