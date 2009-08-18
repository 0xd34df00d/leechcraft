#include "chatter.h"
#include "fsirc.h"
#include <QIcon>
#include <plugininterface/util.h>

using namespace LeechCraft::Plugins::Chatter;

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
}

void Plugin::Release ()
{
	qDeleteAll(Actions_);
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

Q_EXPORT_PLUGIN2 (leechcraft_chatter, Plugin);

