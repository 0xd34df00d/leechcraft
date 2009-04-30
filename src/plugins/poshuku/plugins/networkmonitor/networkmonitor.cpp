#include "networkmonitor.h"
#include <typeinfo>

using namespace LeechCraft::Poshuku;
using namespace LeechCraft::Poshuku::Plugins;
using namespace LeechCraft::Poshuku::Plugins::NetworkMonitor;

void Plugin::Init ()
{
}

void Plugin::Release ()
{
}

QString Plugin::GetName () const
{
	return "Poshuku NetworkMonitor";
}

QString Plugin::GetInfo () const
{
	return tr ("Monitors network activity.");
}

QIcon Plugin::GetIcon () const
{
	return QIcon ();
}

QStringList Plugin::Provides () const
{
	return QStringList ();
}

QStringList Plugin::Needs () const
{
	return QStringList ("webbrowser");
}

QStringList Plugin::Uses () const
{
	return QStringList ();
}

void Plugin::SetProvider (QObject*, const QString&)
{
}

QByteArray Plugin::GetPluginClass () const
{
	return QByteArray (typeid (LeechCraft::Poshuku::PluginBase).name ());
}

void Plugin::Init (IProxyObject*)
{
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_networkmonitor, Plugin);

