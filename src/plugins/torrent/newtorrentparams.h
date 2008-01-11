#ifndef NEWTORRENTPARAMS_H
#define NEWTORRENTPARAMS_H
#include <QDate>
#include <QString>
#include <QStringList>

struct NewTorrentParams
{
	QString OutputDirectory_
		, TorrentName_
		, AnnounceURL_
		, Comment_
		, Path_;
	QDate Date_;
	int PieceSize_;
	QStringList URLSeeds_;
	QStringList DHTNodes_;
	bool DHTEnabled_;
};

#endif

