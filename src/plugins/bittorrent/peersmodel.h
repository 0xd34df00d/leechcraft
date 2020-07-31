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
#include <QList>
#include "peerinfo.h"

namespace LC
{
namespace BitTorrent
{
	class PeersModel : public QAbstractItemModel
	{
		Q_OBJECT

		QStringList Headers_;
		QList<PeerInfo> Peers_;
		int CurrentTorrent_;
		const int Index_;

		QString FlagsPath_;
	public:
		enum { SortRole = Qt::UserRole + 1 };

		PeersModel (int idx, QObject *parent = 0);

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual bool hasChildren (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex ()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

		const PeerInfo& GetPeerInfo (const QModelIndex&) const;
	public slots:
		void update ();
	private:
		void Clear ();
		void Update (const QList<PeerInfo>&);
	};
}
}
