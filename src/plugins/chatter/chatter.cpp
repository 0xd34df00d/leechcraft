#include "chatter.h"
#include "fsirc.h"
#include <QIcon>
#include "core.h"
#include "findproxy.h"
#include <plugininterface/util.h>

using namespace LeechCraft::Plugins::Chatter;

void Plugin::Init (ICoreProxy_ptr proxy)
{
	Q_UNUSED(proxy);
	fsIrc = new fsirc();
	QAction *showAction = new QAction (tr ("Chatter..."),
			this);
	showAction->setProperty ("ActionIcon", "chatter_plugin");
	connect (showAction,
			SIGNAL (triggered ()),
			fsIrc,
			SLOT (show ()));
	Actions_.push_back (showAction);

	SettingsDialog_.reset (new Util::XmlSettingsDialog ());
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), "chattersettings.xml");
	if(XmlSettingsManager::Instance()->property("ShowTrayIcon").toBool())
		fsIrc->addTrayIcon();
	SettingsDialog_->RegisterObject ("ShowTrayIcon", fsIrc, "setTrayPresence");
}

void Plugin::Release ()
{
	Core::Instance ().Release ();
	qDeleteAll(Actions_);
}

QString Plugin::GetName () const
{
	return "IRC client";
}

QString Plugin::GetInfo () const
{
	return tr ("Allows to chat");
}

QIcon Plugin::GetIcon () const
{
	return QIcon ();
}

QStringList Plugin::Provides () const
{
	return QStringList ("irc");
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
	return QStringList ("irc");
}

IFindProxy_ptr Plugin::GetProxy (const LeechCraft::Request& r)
{
	return IFindProxy_ptr (new FindProxy (r));
}

QList<QAction*> Plugin::GetActions () const
{
	return Actions_;
}

void Plugin::Handle (LeechCraft::DownloadEntity)
{

}

boost::shared_ptr<Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
{
	return SettingsDialog_;
}

Q_EXPORT_PLUGIN2 (leechcraft_chatter, Plugin);

