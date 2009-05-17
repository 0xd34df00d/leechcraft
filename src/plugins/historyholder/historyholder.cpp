#include "historyholder.h"
#include "core.h"
#include "findproxy.h"

using namespace LeechCraft::Plugins::HistoryHolder;

void Plugin::Init (ICoreProxy_ptr)
{
}

void Plugin::Release ()
{
	Core::Instance ().Release ();
}

QString Plugin::GetName () const
{
	return "History holder";
}

QString Plugin::GetInfo () const
{
	return tr ("Holds history from various plugins");
}

QIcon Plugin::GetIcon () const
{
	return QIcon (":/resources/images/historyholder.png");
}

QStringList Plugin::Provides () const
{
	return QStringList ("history");
}

QStringList Plugin::Needs () const
{
	return QStringList ();
}

QStringList Plugin::Uses () const
{
	return QStringList ();
}

void Plugin::SetProvider (QObject*, const QString&)
{
}

QStringList Plugin::GetCategories () const
{
	return QStringList ("history");
}

IFindProxy_ptr Plugin::GetProxy (const LeechCraft::Request& r)
{
	return IFindProxy_ptr (new FindProxy (r));
}

bool Plugin::CouldHandle (const LeechCraft::DownloadEntity& e) const
{
	Core::Instance ().Handle (e);
	return false;
}

void Plugin::Handle (LeechCraft::DownloadEntity)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, Plugin);

