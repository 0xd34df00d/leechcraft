#include "core.h"
#include <QtDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include "adaptor.h"

Core::Core ()
{
	new Adaptor (this);

	QDBusConnection::sessionBus ().registerService ("org.LeechCraft.DBus");
	QDBusConnection::sessionBus ().registerObject ("/org/LeechCraft/Manager", this);
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	emit aboutToQuit ();
}

QString Core::Greeter (const QString& msg)
{
	return "LeechCraft D-Bus connector development version";
}

QStringList Core::GetLoadedPlugins ()
{
	return QStringList ("Not implemented");
}

void Core::DumpError ()
{
	qDebug () << Q_FUNC_INFO
		<< Connection_->lastError ().message ();
}

