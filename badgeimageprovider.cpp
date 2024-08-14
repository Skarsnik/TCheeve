#include <QLoggingCategory>
#include "badgeimageprovider.h"

Q_LOGGING_CATEGORY(log_badgeImageProvider, "BadgeImageProvider")
#define sDebug() qCDebug(log_badgeImageProvider)
#define sInfo() qCInfo(log_badgeImageProvider)

BadgeImageProvider::BadgeImageProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

QPixmap BadgeImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    sDebug() << "Querying image " << id;
    if (badgePixmaps.isEmpty())
    {
        if (id.endsWith("_lock"))
            return QPixmap(":/img/mykottShicken.png");
        return QPixmap(":/img/kottshicken.png");
    }
    sDebug() << "Getting " << id;
    return badgePixmaps[id];
}

void BadgeImageProvider::addBadgePixmap(const QString id, QByteArray data)
{
    sInfo() << "Added image " << id;
    badgePixmaps[id] = QPixmap::fromImage(QImage::fromData(data));
}
