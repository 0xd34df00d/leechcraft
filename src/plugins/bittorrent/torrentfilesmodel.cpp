/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "torrentfilesmodel.h"
#include <QUrl>
#include <QTimer>
#include <QtDebug>
#include <libtorrent/alert_types.hpp>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/unreachable.h>
#include <util/sys/extensionsdata.h>
#include <util/models/modelitembase.h>
#include <interfaces/core/ientitymanager.h>
#include "alertdispatcher.h"
#include "core.h"
#include "cachedstatuskeeper.h"
#include "ltutils.h"

namespace LC::BitTorrent
{
	TorrentFilesModel::TorrentFilesModel (const libtorrent::torrent_handle& handle, AlertDispatcher& dispatcher)
	: TorrentFilesModelBase { { tr ("Name"), tr ("Priority"), tr ("Progress") } }
	, Handle_ { std::make_unique<libtorrent::torrent_handle> (handle) }
	{
		auto timer = new QTimer (this);
		timer->callOnTimeout (this, &TorrentFilesModel::Update);
		timer->start (2000);

		Update ();

		RegGuard_ = dispatcher.RegisterTemporaryHandler ([this] (const libtorrent::file_renamed_alert& a)
				{
					if (a.handle == *Handle_)
						HandleFileRenamed (a.index, QString::fromUtf8 (a.new_name ()));
				});
	}

	TorrentFilesModel::~TorrentFilesModel () = default;

	QVariant TorrentFilesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto node = static_cast<TorrentNodeInfo*> (index.internalPointer ());
		switch (role)
		{
		case Qt::CheckStateRole:
			if (index.column () != ColumnPath)
				return {};

			if (node->Priority_ > 0)
				return Qt::Checked;
			if (node->Priority_ < 0)
				return Qt::PartiallyChecked;
			return Qt::Unchecked;
		case Qt::DisplayRole:
			switch (index.column ())
			{
			case ColumnPath:
				return node->Name_;
			case ColumnPriority:
				return node->Priority_ >= 0 ?
						node->Priority_ :
						QVariant {};
			case ColumnProgress:
				return node->Progress_;
			}
			Util::Unreachable ();
		case Qt::DecorationRole:
			return index.column () == ColumnPath ?
					node->Icon_ :
					QIcon {};
		case RoleFullPath:
			return node->GetFullPathStr ();
		case RoleFileName:
			return node->Name_;
		case RoleProgress:
			return std::max ({}, node->Progress_);
		case RoleSize:
			return node->SubtreeSize_;
		case RolePriority:
			return node->Priority_;
		case RoleSort:
			switch (index.column ())
			{
			case ColumnPath:
				return node->Name_;
			case ColumnPriority:
				return node->Priority_;
			case ColumnProgress:
				return node->Progress_;
			}
			Util::Unreachable ();
		}
		return {};
	}

	Qt::ItemFlags TorrentFilesModel::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled };
		switch (index.column ())
		{
		case ColumnPath:
			flags |= Qt::ItemIsEditable;
			flags |= Qt::ItemIsUserCheckable;
			break;
		case ColumnPriority:
			flags |= Qt::ItemIsEditable;
			break;
		}
		return flags;
	}

	bool TorrentFilesModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid ())
			return false;

		const auto node = static_cast<TorrentNodeInfo*> (index.internalPointer ());
		switch (index.column ())
		{
		case ColumnPriority:
			if (const auto rc = rowCount (index))
				for (int i = 0; i < rc; ++i)
					setData (this->index (i, index.column (), index), value, role);
			else
			{
				const auto newPriority = value.toInt ();
				Core::Instance ()->SetFilePriority (node->FileIndex_, newPriority, Index_);
				node->Priority_ = newPriority;
				emit dataChanged (index.sibling (index.row (), ColumnPath), index);

				UpdatePriorities (node);
			}
			return true;
		case ColumnPath:
		{
			switch (role)
			{
			case Qt::EditRole:
			{
				auto newPath = value.toString ();
				const auto& curPath = node->GetFullPathStr ();
				if (curPath.contains ('/') && !newPath.contains ('/'))
				{
					const auto lastIdx = curPath.lastIndexOf ('/');
					newPath = curPath.left (lastIdx + 1) + newPath;
				}

				if (!node->IsEmpty ())
				{
					const auto curPathSize = curPath.size ();
					std::function<void (TorrentNodeInfo*)> setter =
							[this, &setter, &newPath, curPathSize] (TorrentNodeInfo *node)
							{
								if (node->IsEmpty ())
								{
									auto specificPath = node->GetFullPathStr ();
									specificPath.replace (0, curPathSize, newPath);
									Core::Instance ()->SetFilename (node->FileIndex_, specificPath, Index_);
								}
								else
									for (const auto& subnode : *node)
										setter (subnode.get ());
							};
					setter (node);
				}
				else
					Core::Instance ()->SetFilename (node->FileIndex_, newPath, Index_);
				return true;
			}
			case Qt::CheckStateRole:
				return setData (index.sibling (index.row (), ColumnPriority),
						value.toInt () == Qt::Checked ? 1 : 0,
						Qt::EditRole);
			}
		}
		}

		return false;
	}

	void TorrentFilesModel::ResetFiles (const std::filesystem::path& basePath,
			const QList<FileInfo>& infos)
	{
		Clear ();

		BasePath_ = basePath;

		beginInsertRows ({}, 0, 0);
		FilesInTorrent_ = infos.size ();
		Path2Node_ [{}] = RootNode_;

		const auto& inst = Util::ExtensionsData::Instance ();

		for (int i = 0; i < infos.size (); ++i)
		{
			const auto& fi = infos.at (i);
			const auto& parentItem = MkParentIfDoesntExist (fi.Path_);

			const auto& filename = QString::fromStdU16String (fi.Path_.filename ().u16string ());

			const auto item = parentItem->AppendChild (parentItem);
			item->Name_ = filename;
			item->ParentPath_ = fi.Path_.parent_path ();
			item->Priority_ = fi.Priority_;
			item->FileIndex_ = i;
			item->SubtreeSize_ = fi.Size_;
			item->Progress_ = fi.Progress_;
			item->Icon_ = inst.GetExtIcon (filename.section ('.', -1));

			Path2Node_ [fi.Path_] = item;

			UpdatePriorities (item.get ());
		}

		UpdateSizeGraph (RootNode_);

		endInsertRows ();
	}

	void TorrentFilesModel::UpdateFiles (const std::filesystem::path& basePath,
			const QList<FileInfo>& infos)
	{
		BasePath_ = basePath;
		if (Path2Node_.size () <= 1)
		{
			ResetFiles (BasePath_, infos);
			return;
		}

		for (const auto& fi : infos)
		{
			const auto pos = Path2Node_.find (fi.Path_);
			if (pos == Path2Node_.end ())
				continue;

			const auto& item = pos->second;
			item->Progress_ = fi.Progress_;
			item->SubtreeSize_ = fi.Size_;
		}

		UpdateSizeGraph (RootNode_);

		if (const auto rc = RootNode_->GetRowCount ())
			emit dataChanged (index (0, 2), index (rc - 1, 2));
	}

	void TorrentFilesModel::HandleFileActivated (QModelIndex index) const
	{
		if (!index.isValid ())
			return;

		if (index.column () != ColumnPath)
			index = index.sibling (index.row (), ColumnPath);

		const auto item = static_cast<TorrentNodeInfo*> (index.internalPointer ());

		const auto iem = GetProxyHolder ()->GetEntityManager ();
		if (std::abs (item->Progress_ - 1) >= std::numeric_limits<decltype (item->Progress_)>::epsilon ())
			iem->HandleEntity (Util::MakeNotification (QStringLiteral ("BitTorrent"),
					tr ("%1 hasn't finished downloading yet.")
						.arg ("<em>" + item->Name_ + "</em>"),
					Priority::Warning));
		else
		{
			const auto& full = BasePath_ / item->GetFullPath ();
			const auto& path = QString::fromUtf8 (full.string ().c_str ());
			const auto& e = Util::MakeEntity (QUrl::fromLocalFile (path),
					{},
					FromUserInitiated);
			iem->HandleEntity (e);
		}
	}

	void TorrentFilesModel::UpdateSizeGraph (const TorrentNodeInfo_ptr& node)
	{
		if (!node->GetRowCount ())
			return;

		qulonglong size = 0;
		qulonglong done = 0;

		for (const auto & child : *node)
		{
			UpdateSizeGraph (child);
			size += child->SubtreeSize_;
			done += child->SubtreeSize_ * child->Progress_;
		}

		node->SubtreeSize_ = size;
		node->Progress_ = size ? static_cast<double> (done) / size : 1;
	}

	void TorrentFilesModel::UpdatePriorities (TorrentNodeInfo *node)
	{
		const auto& parent = node->GetParent ();
		if (node == RootNode_.get () || parent == RootNode_)
			return;

		const auto prio = node->Priority_;
		const bool allSame = std::all_of (parent->begin (), parent->end (),
				[prio] (const TorrentNodeInfo_ptr& child) { return child->Priority_ == prio; });

		const auto newPrio = allSame ? prio : -1;
		if (newPrio == parent->Priority_)
			return;

		parent->Priority_ = newPrio;
		const auto& idx = IndexForNode (parent, ColumnPriority);
		emit dataChanged (idx.sibling (idx.row (), ColumnPath), idx);

		UpdatePriorities (parent.get ());
	}

	void TorrentFilesModel::ClearEmptyParents (const std::filesystem::path& path)
	{
		const auto pos = Path2Node_.find (path);
		if (pos == Path2Node_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown path"
					<< path.c_str ();
			return;
		}

		const auto& node = pos->second;
		if (!node->IsEmpty ())
		{
			UpdateSizeGraph (RootNode_);
			return;
		}

		const auto& parentNode = node->GetParent ();

		const auto nodeRow = node->GetRow ();
		const auto& parentIndex = FindIndex (path.parent_path ());
		beginRemoveRows (parentIndex, nodeRow, nodeRow);
		parentNode->EraseChild (parentNode->begin () + nodeRow);
		Path2Node_.erase (pos);
		endRemoveRows ();

		ClearEmptyParents (path.parent_path ());
	}

	void TorrentFilesModel::Update ()
	{
		const auto& base = Core::Instance ()->GetStatusKeeper ()->GetStatus (*Handle_, libtorrent::torrent_handle::query_save_path).save_path;

		const auto& files = GetTorrentFiles (*Handle_);
		UpdateFiles (base, files);
	}

	void TorrentFilesModel::HandleFileRenamed (int file, const QString& newName)
	{
		const auto filePos = std::find_if (Path2Node_.begin (), Path2Node_.end (),
				[file] (const Path2Node_t::value_type& pair)
					{ return pair.second->FileIndex_ == file; });
		if (filePos == Path2Node_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown file index"
					<< file
					<< "was renamed to"
					<< newName;
			return;
		}

		const auto node = filePos->second;
		ClearEmptyParents (filePos->first);

		const std::filesystem::path newPath { newName.toStdString () };

		const auto& parentNode = MkParentIfDoesntExist (newPath, true);

		node->Name_ = QString::fromStdU16String (newPath.filename ().u16string ());
		node->Reparent (parentNode);

		beginInsertRows (FindIndex (newPath.parent_path ()), parentNode->GetRowCount (), parentNode->GetRowCount ());
		Path2Node_ [newPath] = node;
		parentNode->AppendExisting (node);
		endInsertRows ();

		UpdateSizeGraph (RootNode_);
	}
}
