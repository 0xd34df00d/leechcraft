#include <QtDebug>
#include <QtCore>
#include "torrentfileparser.h"
#include "torrentmanager.h"

TorrentManager::TorrentManager (QObject *parent)
: QObject (parent)
{
}

QList<QPair<QString, int> > TorrentManager::GetContainedFiles (QString file) const
{
	qDebug () << Q_FUNC_INFO;
	if (file.isEmpty ())
	{
		qDebug () << "Got empty file, returning empty list";
		return QList<QPair<QString, int> > ();
	}

	QFile torrent (file);
	if (!torrent.open (QIODevice::ReadOnly))
	{
		emit showError (tr ("Torrent file<br /><code>%1</code><br />could not be opened. Sorry.").arg (file));
		return QList<QPair<QString, int> > ();
	}
	
	TorrentFileParser tfp;
	bool parseResult = tfp.Parse (torrent.readAll ());
	if (!parseResult)
	{
		emit showError (tr ("Torrent file<br /><code>%1</code><br />could not be parsed. Sorry.").arg (file));
		return QList<QPair<QString, int> > ();
	}

	Dictionary_t info = tfp.GetInfo ();

	QList<QPair<QString, int> > result;

	if (info.find ("files") == info.end ())
		result.append (qMakePair<QString, int> (info ["name"].toString (), info ["length"].value<qint64> ()));
	else
		for (int i = 0; i < info ["files"].toList ().size (); ++i)
		{
			QString path;
			QList<QVariant> pathParts = info ["files"].toList () [i].value<Dictionary_t> () ["path"].toList ();
			for (int j = 0; j < pathParts.size (); ++j)
				path += (pathParts [j].toByteArray () + "/");
			path = path.left (path.size () - 1);
			result.append (qMakePair<QString, int> (path, info ["files"].toList () [i].value<Dictionary_t> () ["length"].value<qint64> ()));
		}


	return result;
}

void TorrentManager::AddJob (const QString& filename, const QStringList& files)
{
}

