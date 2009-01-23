#include "deadlyrics.h"

void DeadLyRicS::Init ()
{
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

QStringList DeadLyRicS::GetCategories () const
{
	return QStringList () << tr ("Lyrics");
}

void DeadLyRicS::Start (const QString&, const QStringList&)
{
}

void DeadLyRicS::Abort ()
{
}

Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, DeadLyRicS);

