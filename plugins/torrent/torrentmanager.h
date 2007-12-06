#ifndef TORRENTMANAGER_H
#define TORRENTMANAGER_H
#include <QObject>
#include <QMap>
#include <QPair>
#include <QList>
#include <QString>

class TorrentManager : public QObject
{
	Q_OBJECT
public:
	TorrentManager (QObject *parent = 0);
signals:
	void showError (QString) const;
};

#endif

