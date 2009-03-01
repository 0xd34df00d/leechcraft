#include "seekthru.h"

void SeekThru::Init ()
{
}

void SeekThru::Release ()
{
}

QString SeekThru::GetName () const
{
	return "SeekThru";
}

QString SeekThru::GetInfo () const
{
	return tr ("Search via OpenSearch-aware search providers.");
}

QIcon SeekThru::GetIcon () const
{
	return QIcon ();
}

QStringList SeekThru::Provides () const
{
	return QStringList ("opensearch");
}

QStringList SeekThru::Needs () const
{
	return QStringList ("http");
}

QStringList SeekThru::Uses () const
{
	return QStringList ();
}

void SeekThru::SetProvider (QObject*, const QString&)
{
}

QStringList SeekThru::GetCategories () const
{
	return QStringList ();
}

boost::shared_ptr<IFindProxy> SeekThru::GetProxy (const LeechCraft::Request&)
{
	return boost::shared_ptr<IFindProxy> ();
}

Q_EXPORT_PLUGIN2 (leechcraft_seekthru, SeekThru);

