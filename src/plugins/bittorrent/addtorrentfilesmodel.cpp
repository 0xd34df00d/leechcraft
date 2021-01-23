/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addtorrentfilesmodel.h"
#include <filesystem>
#include <util/util.h>
#include <util/sll/unreachable.h>

namespace LC::BitTorrent
{
	AddTorrentFilesModel::AddTorrentFilesModel (QObject *parent)
	: TorrentFilesModelBase { { tr ("Name"), tr ("Size") },  parent }
	{
	}

	QVariant AddTorrentFilesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		const auto node = static_cast<AddTorrentNodeInfo*> (index.internalPointer ());
		switch (role)
		{
		case Qt::CheckStateRole:
			return index.column () == ColumnPath ?
					node->CheckState_ :
					QVariant {};
		case Qt::DisplayRole:
			switch (index.column ())
			{
			case ColumnPath:
				return node->Name_;
			case ColumnSize:
				return Util::MakePrettySize (node->SubtreeSize_);
			}
			Util::Unreachable ();
		case Qt::DecorationRole:
			return index.column () == ColumnPath ?
					node->Icon_ :
					QIcon {};
		case RoleSort:
			switch (index.column ())
			{
			case ColumnPath:
				return node->Name_;
			case ColumnSize:
				return node->SubtreeSize_;
			}
			Util::Unreachable ();
		case RoleFullPath:
			return node->GetFullPathStr ();
		case RoleFileName:
			return node->Name_;
		case RoleSize:
			return node->SubtreeSize_;
		}
		return {};
	}

	Qt::ItemFlags AddTorrentFilesModel::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return {};

		auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		if (index.column () == ColumnPath)
			flags |= Qt::ItemIsUserCheckable;
		return flags;
	}

	bool AddTorrentFilesModel::setData (const QModelIndex& index, const QVariant& value, int role)
	{
		if (!index.isValid () || role != Qt::CheckStateRole)
			return false;

		const auto node = static_cast<AddTorrentNodeInfo*> (index.internalPointer ());
		node->CheckState_ = static_cast<Qt::CheckState> (value.toInt ());
		emit dataChanged (index, index);

		for (int i = 0, rc = rowCount (index); i < rc; ++i)
			setData (this->index (i, 0, index), value, role);

		auto pi = parent (index);
		while (pi.isValid ())
		{
			bool hasChecked = false;
			bool hasUnchecked = false;
			const auto prows = rowCount (pi);
			for (int i = 0; i < prows; ++i)
			{
				int state = this->index (i, 0, pi).data (role).toInt ();
				switch (static_cast<Qt::CheckState> (state))
				{
				case Qt::Checked:
					hasChecked = true;
					break;
				case Qt::Unchecked:
					hasUnchecked = true;
					break;
				default:
					hasChecked = true;
					hasUnchecked = true;
					break;
				}
				if (hasChecked && hasUnchecked)
					break;
			}

			auto state = Qt::Unchecked;
			if (hasChecked && hasUnchecked)
				state = Qt::PartiallyChecked;
			else if (hasChecked)
				state = Qt::Checked;
			else if (hasUnchecked)
				state = Qt::Unchecked;
			else
				qWarning () << Q_FUNC_INFO
					<< pi
					<< "we have neither checked nor unchecked items. Strange.";

			const auto parentNode = static_cast<AddTorrentNodeInfo*> (pi.internalPointer ());
			if (parentNode->CheckState_ == state)
				break;

			parentNode->CheckState_ = state;
			emit dataChanged (pi, pi);

			pi = parent (pi);
		}

		return true;
	}

	void AddTorrentFilesModel::ResetFiles (const QList<FileEntry>& entries)
	{
		Clear ();

		FilesInTorrent_ = entries.size ();
		if (!FilesInTorrent_)
			return;

		beginInsertRows ({}, 0, 0);
		Path2Node_ [{}] = RootNode_;

		int fileIdx = 0;
		for (const auto& entry : entries)
		{
			const std::filesystem::path path { entry.Path_ };
			const auto& parentItem = MkParentIfDoesntExist (path);

			const auto& item = parentItem->AppendChild (parentItem);
			item->Name_ = QString::fromStdU16String (path.filename ().u16string ());
			item->ParentPath_ = path.parent_path ();
			item->FileIndex_ = fileIdx++;
			item->SubtreeSize_ = entry.Size_;

			Path2Node_ [path] = item;
		}

		UpdateSizeGraph (RootNode_);

		endInsertRows ();
	}

	QVector<bool> AddTorrentFilesModel::GetSelectedFiles () const
	{
		QVector<bool> result (FilesInTorrent_);
		for (const auto& pair : Path2Node_)
			if (pair.second->FileIndex_ != -1)
				result [pair.second->FileIndex_] = pair.second->CheckState_ == Qt::Checked;
		return result;
	}

	void AddTorrentFilesModel::MarkAll ()
	{
		const auto rc = RootNode_->GetRowCount ();
		if (!rc)
			return;

		for (const auto& pair : Path2Node_)
			pair.second->CheckState_ = Qt::Checked;
		emit dataChanged (index (0, 0), index (rc - 1, 1));
	}

	void AddTorrentFilesModel::UnmarkAll ()
	{
		const auto rc = RootNode_->GetRowCount ();
		if (!rc)
			return;

		for (const auto& pair : Path2Node_)
			pair.second->CheckState_ = Qt::Unchecked;
		emit dataChanged (index (0, 0), index (rc - 1, 1));
	}

	void AddTorrentFilesModel::MarkIndexes (const QList<QModelIndex>& indexes)
	{
		for (const auto& index : indexes)
			setData (index.sibling (index.row (), ColumnPath), Qt::Checked, Qt::CheckStateRole);
	}

	void AddTorrentFilesModel::UnmarkIndexes (const QList<QModelIndex>& indexes)
	{
		for (const auto& index : indexes)
			setData (index.sibling (index.row (), ColumnPath), Qt::Unchecked, Qt::CheckStateRole);
	}

	void AddTorrentFilesModel::UpdateSizeGraph (const AddTorrentNodeInfo_ptr& node)
	{
		if (!node->GetRowCount ())
			return;

		qulonglong size = 0;

		for (const auto & child : *node)
		{
			UpdateSizeGraph (child);
			size += child->SubtreeSize_;
		}

		node->SubtreeSize_ = size;
	}
}
