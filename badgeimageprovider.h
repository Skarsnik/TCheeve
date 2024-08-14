#ifndef BADGEIMAGEPROVIDER_H
#define BADGEIMAGEPROVIDER_H

#include <QQuickImageProvider>

class BadgeImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    BadgeImageProvider();

    // QQuickImageProvider interface
public:
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
    void addBadgePixmap(const QString id, QByteArray data);

private:
    QHash<QString, QPixmap> badgePixmaps;
};

#endif // BADGEIMAGEPROVIDER_H
