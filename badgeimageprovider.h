#ifndef BADGEIMAGEPROVIDER_H
#define BADGEIMAGEPROVIDER_H

#include <QNetworkAccessManager>
#include <QQuickImageProvider>

class BadgeImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    BadgeImageProvider();

    // QQuickImageProvider interface
public:
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
    void    getBadges(QStringList badges);
    void    addBadgePixmap(const QString id, QByteArray data);

signals:
    void    achievementBadgesReady();
private:
    QHash<QString, QPixmap> badgePixmaps;
    QNetworkAccessManager   dlManager;
    unsigned int            imagesToProcessCount;
};

#endif // BADGEIMAGEPROVIDER_H
