#ifndef NETWORKDISKCACHE_H
#define NETWORKDISKCACHE_H
#include <QNetworkDiskCache>

class NetworkDiskCache : public QNetworkDiskCache
{
	Q_OBJECT
public:
	NetworkDiskCache (QObject* = 0);

	virtual QIODevice* prepare (const QNetworkCacheMetaData&);
};

#endif

