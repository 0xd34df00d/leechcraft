#include "chatter.h"
#include "fsirc.h"
#include <QIcon>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"

using namespace LeechCraft::Plugins::Chatter;
using namespace LeechCraft;

void Plugin::Init (ICoreProxy_ptr)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("chatter"));

	fsIrc = new fsirc();
	QAction *showAction = new QAction (GetName (),
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

void Plugin::SecondInit ()
{
}

void Plugin::Release ()
{
	qDeleteAll(Actions_);
	delete fsIrc;
}

QString Plugin::GetName () const
{
	return "Chatter";
}

QString Plugin::GetInfo () const
{
	return tr ("Allows to chat in IRC");
}

QIcon Plugin::GetIcon () const
{
	return QIcon (":/fsirc/data/icon.svg");
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

bool Plugin::CouldHandle (const LeechCraft::DownloadEntity& e) const
{
	if (!e.Entity_.canConvert<QUrl> ())
		return false;
	return (e.Entity_.toUrl().scheme() == "irc");
}

void Plugin::Handle (LeechCraft::DownloadEntity e)
{
	if (!e.Entity_.canConvert<QUrl> ())
		return;
	fsIrc->newTab(e.Entity_.toUrl().toString());
}

boost::shared_ptr<Util::XmlSettingsDialog> Plugin::GetSettingsDialog () const
{
	return SettingsDialog_;
}

Q_EXPORT_PLUGIN2 (leechcraft_chatter, Plugin);

