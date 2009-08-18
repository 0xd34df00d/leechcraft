#include "chatter.h"
#include "fsirc.h"
#include <QIcon>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

using namespace LeechCraft::Plugins::Chatter;
using namespace LeechCraft;

void Plugin::Init (ICoreProxy_ptr proxy)
{
	Q_UNUSED(proxy);
	fsIrc = new fsirc();
	QAction *showAction = new QAction (tr ("Chatter..."),
			this);
	showAction->setIcon (QIcon (":/fsirc/data/icon.svg"));
	connect (showAction,
			SIGNAL (triggered ()),
			fsIrc,
			SLOT (show ()));
	Actions_.push_back (showAction);

	SettingsDialog_.reset (new Util::XmlSettingsDialog ());
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), "chattersettings.xml");
	if(XmlSettingsManager::Instance()->property("ShowTrayIcon").toBool())
		fsIrc->addTrayIcon();
	XmlSettingsManager::Instance()->RegisterObject (QByteArray("ShowTrayIcon"), fsIrc, QByteArray("setTrayPresence"));
}

void Plugin::Release ()
{
	qDeleteAll(Actions_);
	delete fsIrc;
}

QString Plugin::GetName () const
{
	return tr ("IRC client");
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

