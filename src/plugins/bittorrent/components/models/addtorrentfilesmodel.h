/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "torrentfilesmodelbase.h"

namespace LC::BitTorrent
{
	struct AddTorrentNodeInfo : TorrentNodeInfoBase<AddTorrentNodeInfo>
	{
		Qt::CheckState CheckState_ = Qt::Checked;

		using TorrentNodeInfoBase<AddTorrentNodeInfo>::TorrentNodeInfoBase;
	};
	using AddTorrentNodeInfo_ptr = std::shared_ptr<AddTorrentNodeInfo>;

	class AddTorrentFilesModel final : public TorrentFilesModelBase<AddTorrentNodeInfo>
	{
	public:
		enum
		{
			RoleFullPath = Qt::UserRole + 1,
			RoleFileName,
			RoleSize,
			RoleSort
		};

		enum
		{
			ColumnPath,
			ColumnSize
		};

		struct FileEntry
		{
			std::string Path_;
			int64_t Size_;
		};

		explicit AddTorrentFilesModel (QObject *parent = nullptr);

		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole) override;

		void ResetFiles (const QList<FileEntry>&);
		QVector<bool> GetSelectedFiles () const;
		void MarkAll ();
		void UnmarkAll ();
		void MarkIndexes (const QList<QModelIndex>&);
		void UnmarkIndexes (const QList<QModelIndex>&);

		void UpdateSizeGraph (const AddTorrentNodeInfo_ptr& node);
	};
}
