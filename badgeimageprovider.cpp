#include <QFileInfo>
#include <QLoggingCategory>
#include <QNetworkReply>
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

void BadgeImageProvider::getBadges(QStringList badges)
{
    imagesToProcessCount = badges.size();
    for (const auto& url : badges)
    {
        QNetworkReply* reply = dlManager.get(QNetworkRequest(url));
        connect(reply, &QNetworkReply::finished, this, [=] {
            QByteArray imageData = reply->readAll();
            QString badgeId = QFileInfo(reply->url().fileName()).baseName();
            sDebug() << "Received data for " << badgeId << imageData.size();
            addBadgePixmap(badgeId, imageData);
            reply->deleteLater();
            imagesToProcessCount--;
            if (imagesToProcessCount == 0)
                emit achievementBadgesReady();
        });
    }
}

void BadgeImageProvider::addBadgePixmap(const QString id, QByteArray data)
{
    sInfo() << "Added image " << id;
    badgePixmaps[id] = QPixmap::fromImage(QImage::fromData(data));
}
