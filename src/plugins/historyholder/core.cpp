#include "core.h"
#include <typeinfo>
#include <QSettings>
#include <QTextCodec>
#include <QToolBar>
#include <QAction>
#include <QTreeView>
#include <QMainWindow>
#include <plugininterface/structuresops.h>
#include <plugininterface/proxy.h>
#include "findproxy.h"

using LeechCraft::Util::Proxy;
using namespace LeechCraft::Plugins::HistoryHolder;

QDataStream& operator<< (QDataStream& out, const Core::HistoryEntry& e)
{
	quint16 version = 1;
	out << version;

	out << e.Entity_
		<< e.DateTime_;

	return out;
}

QDataStream& operator>> (QDataStream& in, Core::HistoryEntry& e)
{
	quint16 version;
	in >> version;
	if (version == 1)
	{
		in >> e.Entity_
			>> e.DateTime_;
	}
	else
	{
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< version;
	}
	return in;
}

LeechCraft::Plugins::HistoryHolder::Core::Core ()
: ToolBar_ (new QToolBar)
{
	Headers_ << tr ("Entity/location")
			<< tr ("Date")
			<< tr ("Tags");

	qRegisterMetaType<HistoryEntry> ("LeechCraft::Plugins::HistoryHolder::Core::HistoryEntry");
	qRegisterMetaTypeStreamOperators<HistoryEntry> ("LeechCraft::Plugins::HistoryHolder::Core::HistoryEntry");

	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_HistoryHolder");
	int size = settings.beginReadArray ("History");
	for (int i = 0; i < size; ++i)
	{
		settings.setArrayIndex (i);
		History_.append (settings.value ("Item").value<HistoryEntry> ());
	}
	settings.endArray ();

	Remove_ = ToolBar_->addAction (tr ("Remove"),
			this,
			SLOT (remove ()));
	Remove_->setProperty ("ActionIcon", "remove");
}

Core& Core::Instance ()
{
	static Core core;
	return core;
}

void Core::Release ()
{
	ToolBar_.reset ();
}

void Core::SetCoreProxy (ICoreProxy_ptr proxy)
{
	CoreProxy_ = proxy;
	Remove_->setParent (proxy->GetMainWindow ());
	connect (CoreProxy_->GetMainView (),
			SIGNAL (activated (const QModelIndex&)),
			this,
			SLOT (handleActivated (const QModelIndex&)));
}

ICoreProxy_ptr Core::GetCoreProxy () const
{
	return CoreProxy_;
}

void Core::Handle (const LeechCraft::DownloadEntity& entity)
{
	if (entity.Parameters_ & LeechCraft::DoNotSaveInHistory ||
			entity.Parameters_ & LeechCraft::IsntDownloaded)
		return;

	HistoryEntry entry =
	{
		entity,
		QDateTime::currentDateTime ()
	};

	beginInsertRows (QModelIndex (), 0, 0);
	History_.prepend (entry);
	endInsertRows ();

	WriteSettings ();
}

void Core::SetShortcut (int id, const QKeySequence& seq)
{
	switch (id)
	{
		case SRemove:
			Remove_->setShortcut (seq);
			break;
	}
}

QMap<int, LeechCraft::ActionInfo> Core::GetActionInfo () const
{
	QMap<int, ActionInfo> result;
	result [SRemove] = ActionInfo (Remove_->text (),
			Remove_->shortcut (), Remove_->icon ());
	return result;
}

int Core::columnCount (const QModelIndex&) const
{
	return Headers_.size ();
}

QVariant Core::data (const QModelIndex& index, int role) const
{
	int row = index.row ();
	HistoryEntry e = History_.at (row);
	if (role == Qt::DisplayRole)
	{
		switch (index.column ())
		{
			case 0:
				{
					QString entity;
					if (e.Entity_.Entity_.size () < 150)
						entity = QTextCodec::codecForName ("UTF-8")->
							toUnicode (e.Entity_.Entity_);
					else
						entity = tr ("Binary data");
					if (!e.Entity_.Location_.isEmpty ())
					{
						entity += " (";
						entity += e.Entity_.Location_;
						entity += ")";
					}
					return entity;
				}
			case 1:
				return e.DateTime_;
			case 2:
				return data (index, RoleTags).toStringList ().join ("; ");
			default:
				return QVariant ();
		}
	}
	else if (role == RoleTags)
//		TODO
//		return History_.at (row).Entity_
		return QStringList ();
	else if (role == RoleControls)
		return QVariant::fromValue<QToolBar*> (ToolBar_.get ());
	else if (role == RoleHash)
		return e.Entity_.Entity_;
	else if (role == RoleMime)
		return e.Entity_.Mime_;
	else
		return QVariant ();
}

QVariant Core::headerData (int section, Qt::Orientation orient, int role) const
{
	if (orient != Qt::Horizontal ||
			role != Qt::DisplayRole)
		return QVariant ();

	return Headers_ [section];
}

QModelIndex Core::index (int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid () ||
			!hasIndex (row, column, parent))
		return QModelIndex ();

	return createIndex (row, column);
}

QModelIndex Core::parent (const QModelIndex&) const
{
	return QModelIndex ();
}

int Core::rowCount (const QModelIndex& index) const
{
	return index.isValid () ? 0 : History_.size ();
}

void Core::WriteSettings ()
{
	QSettings settings (Proxy::Instance ()->GetOrganizationName (),
			Proxy::Instance ()->GetApplicationName () + "_HistoryHolder");
	settings.beginWriteArray ("History");
	settings.remove ("");
	int i = 0;
	Q_FOREACH (HistoryEntry e, History_)
	{
		settings.setArrayIndex (i++);
		settings.setValue ("Item", QVariant::fromValue<HistoryEntry> (e));
	}
	settings.endArray ();
}

void Core::remove ()
{
	QModelIndex index = CoreProxy_->GetMainView ()->
		selectionModel ()->currentIndex ();
	index = CoreProxy_->MapToSource (index);
	if (!index.isValid ())
	{
		qWarning () << Q_FUNC_INFO
			<< "invalid index"
			<< index;
		return;
	}
	
	const FindProxy *sm = qobject_cast<const FindProxy*> (index.model ());
	if (!sm)
		return;

	index = sm->mapToSource (index);

	beginRemoveRows (QModelIndex (), index.row (), index.row ());
	History_.removeAt (index.row ());
	endRemoveRows ();

	WriteSettings ();
}

void Core::handleActivated (const QModelIndex& si)
{
	QModelIndex index = CoreProxy_->MapToSource (si);
	if (!index.isValid ())
	{
		qWarning () << Q_FUNC_INFO
			<< "invalid index"
			<< index;
		return;
	}
	
	const FindProxy *sm = qobject_cast<const FindProxy*> (index.model ());
	if (!sm)
		return;

	index = sm->mapToSource (index);

	LeechCraft::DownloadEntity e = History_.at (index.row ()).Entity_;
	e.Parameters_ |= LeechCraft::IsntDownloaded;
	e.Parameters_ |= LeechCraft::FromUserInitiated;
	emit gotEntity (e);
}

