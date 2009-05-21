#include "core.h"
#include <boost/bind.hpp>
#include <QtDebug>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QApplication>
#include "adaptor.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::DBusManager;

Core::Core ()
: NotificationManager_ (new NotificationManager)
{
	new Adaptor (this);

	QDBusConnection::sessionBus ().registerService ("org.LeechCraft.DBus");
	QDBusConnection::sessionBus ().registerObject ("/LeechCraft/Manager", this);
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	emit aboutToQuit ();
	Proxy_.reset ();
}

void Core::SetProxy (ICoreProxy_ptr proxy)
{
	Proxy_ = proxy;
	Proxy_->RegisterHook (HookSignature<HIDDownloadFinishedNotification>::Signature_t (
				boost::bind (&NotificationManager::HandleFinishedNotification,
				NotificationManager_.get (),
				_1,
				_2,
				_3)));
}

ICoreProxy_ptr Core::GetProxy () const
{
	return Proxy_;
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

