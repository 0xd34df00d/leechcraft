#include "dbusmanager.h"
#include "core.h"

void DBusManager::Init ()
{
	Core::Instance ();
}

void DBusManager::Release ()
{
}

QString DBusManager::GetName () const
{
	return "DBus Manager";
}

QString DBusManager::GetInfo () const
{
	return "DBus server application for LeechCraft";
}

IInfo& DBusManager::SetID (unsigned long id)
{
	ID_ = id;
	return *this;
}

unsigned long DBusManager::GetID () const
{
	return ID_;
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

Q_EXPORT_PLUGIN2 (leechcraft_dbusmanager, DBusManager);

