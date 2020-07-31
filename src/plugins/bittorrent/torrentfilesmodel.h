/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <libtorrent/torrent_info.hpp>
#include <interfaces/structures.h>
#include "fileinfo.h"
#include "torrentfilesmodelbase.h"

namespace LC
{
namespace BitTorrent
{
	struct TorrentNodeInfo : public TorrentNodeInfoBase<TorrentNodeInfo>
	{
		int Priority_ = -1;
		float Progress_ = 0;

		using TorrentNodeInfoBase<TorrentNodeInfo>::TorrentNodeInfoBase;
	};
	typedef std::shared_ptr<TorrentNodeInfo> TorrentNodeInfo_ptr;

	class TorrentFilesModel : public TorrentFilesModelBase<TorrentNodeInfo>
	{
		Q_OBJECT

		const int Index_ = -1;
	public:
		enum
		{
			RoleFullPath = Qt::UserRole + 1,
			RoleFileName,
			RoleSize,
			RoleProgress,
			RolePriority,
			RoleSort,
		};

		enum
		{
			ColumnPath,
			ColumnPriority,
			ColumnProgress,
		};

		TorrentFilesModel (int);

		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole) override;

		void ResetFiles (const std::filesystem::path&, const QList<FileInfo>&);
		void UpdateFiles (const std::filesystem::path&, const QList<FileInfo>&);

		void HandleFileActivated (QModelIndex) const;
	private:
		void UpdateSizeGraph (const TorrentNodeInfo_ptr&);
		void UpdatePriorities (TorrentNodeInfo*);
		void ClearEmptyParents (std::filesystem::path);
	public slots:
		void update ();
		void handleFileRenamed (int, int, const QString&);
	};
}
}
