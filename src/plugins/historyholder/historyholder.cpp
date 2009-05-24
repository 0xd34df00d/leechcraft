#include "historyholder.h"
#include <QIcon>
#include "core.h"
#include "findproxy.h"

using namespace LeechCraft::Plugins::HistoryHolder;

void Plugin::Init (ICoreProxy_ptr proxy)
{
	Core::Instance ().SetCoreProxy (proxy);
	connect (&Core::Instance (),
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
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

void Plugin::SetShortcut (int id, const QKeySequence& seq)
{
	Core::Instance ().SetShortcut (id, seq);
}

QMap<int, LeechCraft::ActionInfo> Plugin::GetActionInfo () const
{
	return Core::Instance ().GetActionInfo ();
}

Q_EXPORT_PLUGIN2 (leechcraft_deadlyrics, Plugin);

