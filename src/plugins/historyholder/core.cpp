/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "core.h"
#include <typeinfo>
#include <QUrl>
#include <QSettings>
#include <QTextCodec>
#include <QToolBar>
#include <QAction>
#include <QTreeView>
#include <QMainWindow>
#include <QCoreApplication>
#include <plugininterface/structuresops.h>
#include <plugininterface/util.h>
#include "findproxy.h"

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

	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_HistoryHolder");
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
	connect (CoreProxy_->GetTreeViewReemitter (),
			SIGNAL (activated (const QModelIndex&, QTreeView*)),
			this,
			SLOT (handleActivated (const QModelIndex&, QTreeView*)));
}

ICoreProxy_ptr Core::GetCoreProxy () const
{
	return CoreProxy_;
}

void Core::Handle (const LeechCraft::DownloadEntity& entity)
{
	if (entity.Parameters_ & LeechCraft::DoNotSaveInHistory ||
			entity.Parameters_ & LeechCraft::Internal ||
			!(entity.Parameters_ & LeechCraft::IsDownloaded))
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
					QString stren;
					if (e.Entity_.Entity_.canConvert<QUrl> ())
						stren = e.Entity_.Entity_.toUrl ().toString ();
					else if (e.Entity_.Entity_.canConvert<QByteArray> ())
					{
						QByteArray entity = e.Entity_.Entity_.toByteArray ();
						if (entity.size () < 250)
							stren = QTextCodec::codecForName ("UTF-8")->
								toUnicode (entity);
					}
					else
						stren = tr ("Binary data");

					if (!e.Entity_.Location_.isEmpty ())
					{
						stren += " (";
						stren += e.Entity_.Location_;
						stren += ")";
					}
					return stren;
				}
			case 1:
				return e.DateTime_;
			case 2:
				return CoreProxy_->GetTagsManager ()->
					Join (data (index, RoleTags).toStringList ());
			default:
				return QVariant ();
		}
	}
	else if (role == RoleTags)
	{
		QStringList tagids = History_.at (row).Entity_.Additional_ [" Tags"].toStringList ();
		QStringList result;
		Q_FOREACH (QString id, tagids)
			result << CoreProxy_->GetTagsManager ()->GetTag (id);
		return result;
	}
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
	QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_HistoryHolder");
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
	QTreeView *currentView = CoreProxy_->GetCurrentView ();
	if (!currentView)
		return;

	QModelIndexList selected = currentView->selectionModel ()->selectedRows ();
	QList<int> rows;
	Q_FOREACH (QModelIndex index, selected)
	{
		index = CoreProxy_->MapToSource (index);
		if (!index.isValid ())
		{
			qWarning () << Q_FUNC_INFO
				<< "invalid index"
				<< index;
			continue;
		}
		
		const FindProxy *sm = qobject_cast<const FindProxy*> (index.model ());
		if (!sm)
			continue;

		index = sm->mapToSource (index);

		rows << index.row ();
	}

	qSort (rows.begin (), rows.end (), qGreater<int> ());
	Q_FOREACH (int row, rows)
	{
		beginRemoveRows (QModelIndex (), row, row);
		History_.removeAt (row);
		endRemoveRows ();
	}

	WriteSettings ();
}

void Core::handleActivated (const QModelIndex& si, QTreeView*)
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
	e.Parameters_ |= LeechCraft::FromUserInitiated;
	e.Parameters_ &= ~LeechCraft::IsDownloaded;
	emit gotEntity (e);
}

