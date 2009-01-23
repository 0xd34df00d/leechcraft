#include "deadlyrics.h"
#include "core.h"

void DeadLyRicS::Init ()
{
	Core::Instance ().Start ("Tool - Bottom");
}

void DeadLyRicS::Release ()
{
}

QString DeadLyRicS::GetName () const
{
	return "DeadLyRicS";
}

QString DeadLyRicS::GetInfo () const
{
	return tr ("Lyrics Searcher");
}

QIcon DeadLyRicS::GetIcon () const
{
	return QIcon ();
}

QStringList DeadLyRicS::Provides () const
{
	return QStringList ("search::lyrics");
}

QStringList DeadLyRicS::Needs () const
{
	return QStringList ();
}

QStringList DeadLyRicS::Uses () const
{
	return QStringList ();
}

void DeadLyRicS::SetProvider (QObject*, const QString&)
{
}

void DeadLyRicS::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	Core::Instance ().SetNetworkAccessManager (manager);
}

QStringList DeadLyRicS::GetCategories () const
{
	return QStringList () << tr ("Lyrics");
}

void DeadLyRicS::Start (const QString& string,
		const QStringList& categories, bool caseSensitive)
{
	Q_UNUSED (categories);
	Q_UNUSED (caseSensitive);
	Core::Instance ().Start (string);
}

void DeadLyRicS::Abort ()
{
	Core::Instance ().Abort ();
}

Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, DeadLyRicS);

