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
#include "peerinfo.h"

namespace LC::BitTorrent
{
	class PeersModel : public QAbstractItemModel
	{
		Q_OBJECT

		QStringList Headers_;
		QList<PeerInfo> Peers_;
		const int Index_;

		QString FlagsPath_;
	public:
		enum
		{
			SortRole = Qt::UserRole + 1,
			PeerInfoRole
		};

		explicit PeersModel (int idx, QObject *parent = nullptr);

		int columnCount (const QModelIndex& = {}) const override;
		QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
		Qt::ItemFlags flags (const QModelIndex&) const override;
		bool hasChildren (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		int rowCount (const QModelIndex& parent = {}) const override;
	public slots:
		void update ();
	private:
		void Clear ();
		void Update (const QList<PeerInfo>&);
	};
}
