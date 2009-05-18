#include "dbusmanager.h"
#include <QIcon>
#include "core.h"

using namespace LeechCraft::Plugins::DBusManager;

void DBusManager::Init (ICoreProxy_ptr proxy)
{
	Core::Instance ().SetProxy (proxy);
}

void DBusManager::Release ()
{
	Core::Instance ().Release ();
}

QString DBusManager::GetName () const
{
	return "DBus Manager";
}

QString DBusManager::GetInfo () const
{
	return "DBus server application for LeechCraft";
}

QStringList DBusManager::Provides () const
{
	return QStringList ("dbus");
}

QStringList DBusManager::Uses () const
{
	return QStringList ();
}

QStringList DBusManager::Needs () const
{
	return QStringList ();
}

void DBusManager::SetProvider (QObject*, const QString&)
{
}

QIcon DBusManager::GetIcon () const
{
	return QIcon ();
}

Q_EXPORT_PLUGIN2 (leechcraft_dbusmanager, LeechCraft::Plugins::DBusManager::DBusManager);

