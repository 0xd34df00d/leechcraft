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
#include <vector>
#include <libtorrent/torrent_handle.hpp>

namespace LC
{
namespace BitTorrent
{
	class PiecesModel : public QAbstractItemModel
	{
		Q_OBJECT

		QStringList Headers_;
		struct Info
		{
			int Index_;
			int FinishedBlocks_;
			int TotalBlocks_;

			bool operator== (const Info&) const;
		};
		QList<Info> Pieces_;

		const int Index_;
	public:
		PiecesModel (int, QObject *parent = 0);

		virtual int columnCount (const QModelIndex&) const;
		virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual bool hasChildren (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
	public slots:
		void update ();
	private:
		void Clear ();
		void Update (const std::vector<libtorrent::partial_piece_info>&);
	};
}
}
