/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QCoreApplication>
#include "peerinfo.h"

namespace LC::BitTorrent
{
	class GeoIP;

	class PeersModel : public QAbstractItemModel
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::PeersModel)

		const QString FlagsPath_;
		const QStringList Headers_;
		const QModelIndex Index_;

		QList<PeerInfo> Peers_;
	public:
		enum
		{
			SortRole = Qt::UserRole + 1,
			PeerInfoRole
		};

		enum Columns
		{
			ColumnIP,
			ColumnDownloadRate,
			ColumnUploadRate,
			ColumnDownloaded,
			ColumnUploaded,
			ColumnClient,
			ColumnPieces
		};

		explicit PeersModel (const QModelIndex& torrentIdx, QObject *parent = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool hasChildren (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& parent = {}) const override;

		void Update ();
	private:
		void Clear ();
		void Update (const QList<PeerInfo>&);
	};
}
