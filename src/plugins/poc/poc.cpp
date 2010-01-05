#include <QString>
#include <plugininterface/util.h>
#include "poc.h"
#include "editor.h"
#include <QWidget>

using namespace LeechCraft::Plugins::PoC;
using namespace LeechCraft;

void Plugin::Init (ICoreProxy_ptr)
{
	poc = new Editor();
	QAction *showAction = new QAction (GetName (),
			this);
	connect (showAction,
			SIGNAL (triggered ()),
			poc,
			SLOT (show ()));
	Actions_.push_back (showAction);
}

void Plugin::SecondInit ()
{
}

void Plugin::Release ()
{
	qDeleteAll(Actions_);
	delete poc;
}

QString Plugin::GetName () const
{
	return "PoC";
}

QString Plugin::GetInfo () const
{
	return tr ("Edits text");
}

QIcon Plugin::GetIcon () const
{
	return QIcon();
}

QStringList Plugin::Provides () const
{
	return QStringList ("editor");
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

QWidget* Plugin::GetTabContents ()
{
	return poc;
}

QToolBar* Plugin::GetToolBar () const
{
	return 0;
}

Q_EXPORT_PLUGIN2 (leechcraft_chatter, Plugin);

