#include "deadlyrics.h"
#include "core.h"
#include "findproxy.h"

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

void DeadLyRicS::SetNetworkAccessManager (QNetworkAccessManager *manager)
{
	Core::Instance ().SetNetworkAccessManager (manager);
}

QStringList DeadLyRicS::GetCategories () const
{
	return Core::Instance ().GetCategories ();
}

boost::shared_ptr<IFindProxy> DeadLyRicS::GetProxy (const LeechCraft::Request& req)
{
	return boost::shared_ptr<IFindProxy> (new FindProxy (req));
}

Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, DeadLyRicS);

