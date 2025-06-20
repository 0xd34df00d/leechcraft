/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStringList>
#include <util/sll/scopeguards.h>
#include "fileinfo.h"
#include "torrentfilesmodelbase.h"

namespace libtorrent
{
	class torrent_handle;
}

namespace LC::BitTorrent
{
	struct TorrentNodeInfo : TorrentNodeInfoBase<TorrentNodeInfo>
	{
		int Priority_ = -1;
		float Progress_ = 0;

		using TorrentNodeInfoBase::TorrentNodeInfoBase;
	};
	using TorrentNodeInfo_ptr = std::shared_ptr<TorrentNodeInfo>;

	class AlertDispatcher;
	class AlertDispatcherRegGuard;

	class TorrentFilesModel : public TorrentFilesModelBase<TorrentNodeInfo>
	{
		std::unique_ptr<libtorrent::torrent_handle> Handle_;
		Util::DefaultScopeGuard RegGuard_;
	public:
		enum Role
		{
			RoleFullPath = Qt::UserRole + 1,
			RoleFileName,
			RoleSize,
			RoleProgress,
			RolePriority,
			RoleSort,
		};

		enum Column
		{
			ColumnPath,
			ColumnPriority,
			ColumnProgress,
		};

		TorrentFilesModel (const libtorrent::torrent_handle&, AlertDispatcher&);
		~TorrentFilesModel () override;

		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole) override;

		void ResetFiles (const std::filesystem::path&, const QList<FileInfo>&);
		void UpdateFiles (const std::filesystem::path&, const QList<FileInfo>&);

		void HandleFileActivated (QModelIndex) const;
	private:
		void HandleFileRenamed (int, const QString&);

		void Update ();
		void UpdateSizeGraph (const TorrentNodeInfo_ptr&);
		void UpdatePriorities (const TorrentNodeInfo&);
		void ClearEmptyParents (const std::filesystem::path&);
	};
}
